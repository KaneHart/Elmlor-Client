/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2014  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dirs.h"

#include "client.h"
#include "configuration.h"
#include "logger.h"
#include "settings.h"

#include "utils/base64.h"
#include "utils/gettext.h"
#include "utils/mkdir.h"
#include "utils/paths.h"
#include "utils/physfstools.h"

#include "resources/resourcemanager.h"

#ifdef __APPLE__
#include <CoreFoundation/CFBundle.h>
#endif

#ifdef WIN32
#include <SDL_syswm.h>
#include "utils/specialfolder.h"
#endif

#include <sys/stat.h>

#include "debug.h"

#if defined __native_client__
#define _nacl_dir std::string("/persistent/manaplus")
#endif

void Dirs::updateDataPath()
{
    if (settings.options.dataPath.empty()
        && !branding.getStringValue("dataPath").empty())
    {
        if (isRealPath(branding.getStringValue("dataPath")))
        {
            settings.options.dataPath = branding.getStringValue("dataPath");
        }
        else
        {
            settings.options.dataPath = branding.getDirectory().append(
                dirSeparator) + branding.getStringValue("dataPath");
        }
        settings.options.skipUpdate = true;
    }
}

void Dirs::extractDataDir()
{
#ifdef ANDROID
#ifdef USE_SDL2
    Files::setCopyCallBack(&updateProgress);
    setProgress(0);
    extractAssets();

    const std::string zipName = std::string(getenv(
        "APPDIR")).append("/data.zip");
    const std::string dirName = std::string(getenv(
        "APPDIR")).append("/data");
    Files::extractZip(zipName, "data", dirName);
    Files::extractLocale();
#endif
#endif

#if defined __native_client__
    const std::string dirName = _nacl_dir.append("/data");
    Files::extractZip("/http/data.zip", "data", dirName);
#endif
}

void Dirs::mountDataDir()
{
    const ResourceManager *const resman = ResourceManager::getInstance();
    resman->addToSearchPath(PKG_DATADIR "data/perserver/default", false);
    resman->addToSearchPath("data/perserver/default", false);

#if defined __APPLE__
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (uint8_t*)path,
        PATH_MAX))
    {
        fprintf(stderr, "Can't find Resources directory\n");
    }
    CFRelease(resourcesURL);
    // possible crash
    strncat(path, "/data", PATH_MAX - 1);
    resman->addToSearchPath(path, false);
// possible this need for support run client from dmg images.
//    mPackageDir = path;
#endif
    resman->addToSearchPath(PKG_DATADIR "data", false);
    setPackageDir(PKG_DATADIR "data");
    resman->addToSearchPath("data", false);

#ifdef ANDROID
#ifdef USE_SDL2
    if (getenv("APPDIR"))
    {
        const std::string appDir = getenv("APPDIR");
        resman->addToSearchPath(appDir + "/data", false);
        resman->addToSearchPath(appDir + "/data/perserver/default", false);
    }
#endif
#endif

#if defined __native_client__
    resman->addToSearchPath(_nacl_dir.append("/data"), false);
#endif

    // Add branding/data to PhysFS search path
    if (!settings.options.brandingPath.empty())
    {
        std::string path = settings.options.brandingPath;

        // Strip blah.manaplus from the path
#ifdef WIN32
        const int loc1 = path.find_last_of('/');
        const int loc2 = path.find_last_of('\\');
        const int loc = static_cast<int>(std::max(loc1, loc2));
#else
        const int loc = static_cast<int>(path.find_last_of('/'));
#endif
        if (loc > 0)
        {
            resman->addToSearchPath(path.substr(
                0, loc + 1).append("data"), false);
        }
    }
}

void Dirs::initRootDir()
{
    settings.rootDir = PhysFs::getBaseDir();
    const std::string portableName = settings.rootDir + "portable.xml";
    struct stat statbuf;

    if (!stat(portableName.c_str(), &statbuf) && S_ISREG(statbuf.st_mode))
    {
        std::string dir;
        Configuration portable;
        portable.init(portableName);

        if (settings.options.brandingPath.empty())
        {
            branding.init(portableName);
            branding.setDefaultValues(getBrandingDefaults());
        }

        logger->log("Portable file: %s", portableName.c_str());

        if (settings.options.localDataDir.empty())
        {
            dir = portable.getValue("dataDir", "");
            if (!dir.empty())
            {
                settings.options.localDataDir = settings.rootDir + dir;
                logger->log("Portable data dir: %s",
                    settings.options.localDataDir.c_str());
            }
        }

        if (settings.options.configDir.empty())
        {
            dir = portable.getValue("configDir", "");
            if (!dir.empty())
            {
                settings.options.configDir = settings.rootDir + dir;
                logger->log("Portable config dir: %s",
                    settings.options.configDir.c_str());
            }
        }

        if (settings.options.screenshotDir.empty())
        {
            dir = portable.getValue("screenshotDir", "");
            if (!dir.empty())
            {
                settings.options.screenshotDir = settings.rootDir + dir;
                logger->log("Portable screenshot dir: %s",
                    settings.options.screenshotDir.c_str());
            }
        }
    }
}

/**
 * Initializes the home directory. On UNIX and FreeBSD, ~/.mana is used. On
 * Windows and other systems we use the current working directory.
 */
void Dirs::initHomeDir()
{
    initLocalDataDir();
    initTempDir();
    initConfigDir();
}

void Dirs::initLocalDataDir()
{
    settings.localDataDir = settings.options.localDataDir;

    if (settings.localDataDir.empty())
    {
#ifdef __APPLE__
        // Use Application Directory instead of .mana
        settings.localDataDir = std::string(PhysFs::getUserDir()) +
            "/Library/Application Support/" +
            branding.getValue("appName", "Elmlor");
#elif defined __HAIKU__
        settings.localDataDir = std::string(PhysFs::getUserDir()) +
           "/config/data/Mana";
#elif defined WIN32
        settings.localDataDir = getSpecialFolderLocation(CSIDL_LOCAL_APPDATA);
        if (settings.localDataDir.empty())
            settings.localDataDir = std::string(PhysFs::getUserDir());
        settings.localDataDir.append("/Mana");
#elif defined __ANDROID__
        settings.localDataDir = getSdStoragePath() + branding.getValue(
            "appShort", "Elmlor") + "/local";
#elif defined __native_client__
        settings.localDataDir = _nacl_dir.append("/local");
#else
        settings.localDataDir = std::string(PhysFs::getUserDir()) +
            ".local/share/mana";
#endif
    }

    if (mkdir_r(settings.localDataDir.c_str()))
    {
        // TRANSLATORS: directory creation error
        logger->error(strprintf(_("%s doesn't exist and can't be created! "
            "Exiting."), settings.localDataDir.c_str()));
    }
#ifdef USE_PROFILER
    Perfomance::init(settings.localDataDir + "/profiler.log");
#endif
}

void Dirs::initTempDir()
{
    settings.tempDir = settings.localDataDir + dirSeparator + "temp";

    if (mkdir_r(settings.tempDir.c_str()))
    {
        // TRANSLATORS: directory creation error
        logger->error(strprintf(_("%s doesn't exist and can't be created! "
            "Exiting."), settings.tempDir.c_str()));
    }
//    ResourceManager::deleteFilesInDirectory(settings.tempDir);
}

void Dirs::initConfigDir()
{
    settings.configDir = settings.options.configDir;

    if (settings.configDir.empty())
    {
#ifdef __APPLE__
        settings.configDir = settings.localDataDir + dirSeparator
            + branding.getValue("appShort", "mana");
#elif defined __HAIKU__
        settings.configDir = std::string(PhysFs::getUserDir()) +
           "/config/settings/Mana" +
           branding.getValue("appName", "Elmlor");
#elif defined WIN32
        settings.configDir = getSpecialFolderLocation(CSIDL_APPDATA);
        if (settings.configDir.empty())
        {
            settings.configDir = settings.localDataDir;
        }
        else
        {
            settings.configDir.append("/mana/").append(branding.getValue(
                "appShort", "mana"));
        }
#elif defined __ANDROID__
        settings.configDir = getSdStoragePath() + branding.getValue(
            "appShort", "Elmlor").append("/config");
#elif defined __native_client__
        settings.configDir = _nacl_dir.append("/config");
#else
        settings.configDir = std::string(PhysFs::getUserDir()).append(
            "/.config/mana/").append(branding.getValue("appShort", "mana"));
#endif
        logger->log("Generating config dir: " + settings.configDir);
    }

    if (mkdir_r(settings.configDir.c_str()))
    {
        // TRANSLATORS: directory creation error
        logger->error(strprintf(_("%s doesn't exist and can't be created! "
            "Exiting."), settings.configDir.c_str()));
    }
}

/**
 * Parse the update host and determine the updates directory
 * Then verify that the directory exists (creating if needed).
 */
void Dirs::initUpdatesDir()
{
    std::stringstream updates;

    // If updatesHost is currently empty, fill it from config file
    if (settings.updateHost.empty())
        settings.updateHost = config.getStringValue("updatehost");
    if (!checkPath(settings.updateHost))
        return;

    // Don't go out of range int he next check
    if (settings.updateHost.length() < 2)
        return;

    const size_t sz = settings.updateHost.size();
    // Remove any trailing slash at the end of the update host
    if (settings.updateHost.at(sz - 1) == '/')
        settings.updateHost.resize(sz - 1);

    // Parse out any "http://" or "https://", and set the updates directory
    const size_t pos = settings.updateHost.find("://");
    if (pos != settings.updateHost.npos)
    {
        if (pos + 3 < settings.updateHost.length()
            && !settings.updateHost.empty())
        {
            updates << "updates/" << settings.updateHost.substr(pos + 3);
            settings.updatesDir = updates.str();
        }
        else
        {
            logger->log("Error: Invalid update host: %s",
                settings.updateHost.c_str());
            // TRANSLATORS: update server initialisation error
            errorMessage = strprintf(_("Invalid update host: %s."),
                settings.updateHost.c_str());
            client->setState(STATE_ERROR);
        }
    }
    else
    {
        logger->log1("Warning: no protocol was specified for the update host");
        updates << "updates/" << settings.updateHost;
        settings.updatesDir = updates.str();
    }

#ifdef WIN32
    if (settings.updatesDir.find(":") != std::string::npos)
        replaceAll(settings.updatesDir, ":", "_");
#endif

    const std::string updateDir("/" + settings.updatesDir);

    // Verify that the updates directory exists. Create if necessary.
    if (!PhysFs::isDirectory(updateDir.c_str()))
    {
        if (!PhysFs::mkdir(updateDir.c_str()))
        {
#if defined WIN32
            std::string newDir = settings.localDataDir
                + "\\" + settings.updatesDir;
            size_t loc = newDir.find("/", 0);

            while (loc != std::string::npos)
            {
                newDir.replace(loc, 1, "\\");
                loc = newDir.find("/", loc);
            }

            if (!CreateDirectory(newDir.c_str(), nullptr) &&
                GetLastError() != ERROR_ALREADY_EXISTS)
            {
                logger->log("Error: %s can't be made, but doesn't exist!",
                            newDir.c_str());
                // TRANSLATORS: update server initialisation error
                errorMessage = _("Error creating updates directory!");
                client->setState(STATE_ERROR);
            }
#else
            logger->log("Error: %s/%s can't be made, but doesn't exist!",
                settings.localDataDir.c_str(), settings.updatesDir.c_str());
            // TRANSLATORS: update server initialisation error
            errorMessage = _("Error creating updates directory!");
            client->setState(STATE_ERROR);
#endif
        }
    }
    const std::string updateLocal = updateDir + "/local";
    const std::string updateFix = updateDir + "/fix";
    if (!PhysFs::isDirectory(updateLocal.c_str()))
        PhysFs::mkdir(updateLocal.c_str());
    if (!PhysFs::isDirectory(updateFix.c_str()))
        PhysFs::mkdir(updateFix.c_str());
}

void Dirs::initScreenshotDir()
{
    if (!settings.options.screenshotDir.empty())
    {
        settings.screenshotDir = settings.options.screenshotDir;
        if (mkdir_r(settings.screenshotDir.c_str()))
        {
            // TRANSLATORS: directory creation error
            logger->log(strprintf(
                _("Error: %s doesn't exist and can't be created! "
                "Exiting."), settings.screenshotDir.c_str()));
        }
    }
    else if (settings.screenshotDir.empty())
    {
        settings.screenshotDir = decodeBase64String(
            config.getStringValue("screenshotDirectory3"));
        if (settings.screenshotDir.empty())
        {
#ifdef __ANDROID__
            settings.screenshotDir = getSdStoragePath()
                + std::string("/images");

            if (mkdir_r(settings.screenshotDir.c_str()))
            {
                // TRANSLATORS: directory creation error
                logger->log(strprintf(
                    _("Error: %s doesn't exist and can't be created! "
                    "Exiting."), settings.screenshotDir.c_str()));
            }
#else
            settings.screenshotDir = getPicturesDir();
#endif
            if (config.getBoolValue("useScreenshotDirectorySuffix"))
            {
                const std::string configScreenshotSuffix =
                    branding.getValue("screenshots", "Elmlor");

                if (!configScreenshotSuffix.empty())
                {
                    settings.screenshotDir.append(dirSeparator).append(
                        configScreenshotSuffix);
                }
            }
            config.setValue("screenshotDirectory3",
                encodeBase64String(settings.screenshotDir));
        }
    }
    logger->log("screenshotDirectory: " + settings.screenshotDir);
}

void Dirs::initUsersDir()
{
    settings.usersDir = settings.serverConfigDir + "/users/";
    if (mkdir_r(settings.usersDir.c_str()))
    {
        // TRANSLATORS: directory creation error
        logger->error(strprintf(_("%s doesn't exist and can't be created! "
            "Exiting."), settings.usersDir.c_str()));
    }

    settings.npcsDir = settings.serverConfigDir + "/npcs/";
    if (mkdir_r(settings.npcsDir.c_str()))
    {
        // TRANSLATORS: directory creation error
        logger->error(strprintf(_("%s doesn't exist and can't be created! "
            "Exiting."), settings.npcsDir.c_str()));
    }
}
