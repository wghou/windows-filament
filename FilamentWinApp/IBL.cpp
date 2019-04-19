/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "IBL.h"

#include <fstream>
#include <sstream>
#include <string>

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/Skybox.h>

#include <image/KtxBundle.h>
#include <image/KtxUtility.h>

//#include <stb/stb_image.h>

#include <utils/Path.h>
#include <filament/IndirectLight.h>

using namespace filament;
using namespace image;
using namespace filament::math;
using namespace utils;

static constexpr float IBL_INTENSITY = 30000.0f;

IBL::IBL(Engine& engine) : mEngine(engine) {
}

IBL::~IBL() {
    mEngine.destroy(mIndirectLight);
    mEngine.destroy(mTexture);
    mEngine.destroy(mSkybox);
    mEngine.destroy(mSkyboxTexture);
}

bool IBL::loadFromKtx(const std::string& prefix) {
    // First check for compressed variants of the environment.
    Path iblPath(prefix + "_ibl_s3tc.ktx");
    if (!iblPath.exists()) {
        iblPath = Path(prefix + "_ibl.ktx");
        if (!iblPath.exists()) {
            return false;
        }
    }
    Path skyPath(prefix + "_skybox_s3tc.ktx");
    if (!skyPath.exists()) {
        skyPath = Path(prefix + "_skybox.ktx");
        if (!skyPath.exists()) {
            return false;
        }
    }

    auto createKtx = [] (Path path) {
        using namespace std;
        ifstream file(path.getPath(), ios::binary);
        vector<uint8_t> contents((istreambuf_iterator<char>(file)), {});
        return new image::KtxBundle(contents.data(), contents.size());
    };

    KtxBundle* iblKtx = createKtx(iblPath);
    KtxBundle* skyKtx = createKtx(skyPath);

    mSkyboxTexture = KtxUtility::createTexture(&mEngine, skyKtx, false, true);
    mTexture = KtxUtility::createTexture(&mEngine, iblKtx, false, true);

    std::istringstream shstring(iblKtx->getMetadata("sh"));
    for (float3& band : mBands) {
        shstring >> band.x >> band.y >> band.z;
    }

    mIndirectLight = IndirectLight::Builder()
            .reflections(mTexture)
            .irradiance(3, mBands)
            .intensity(IBL_INTENSITY)
            .build(mEngine);

    mSkybox = Skybox::Builder().environment(mSkyboxTexture).showSun(true).build(mEngine);

    return true;
}

bool IBL::loadFromDirectory(const utils::Path& path) {
    // First check if KTX files are available.
    if (loadFromKtx(Path::concat(path, path.getName()))) {
        return true;
    }
    // Read spherical harmonics
    Path sh(Path::concat(path, "sh.txt"));
    if (sh.exists()) {
        std::ifstream shReader(sh);
        shReader >> std::skipws;

        std::string line;
        for (float3& band : mBands) {
            std::getline(shReader, line);
            int n = sscanf(line.c_str(), "(%f,%f,%f)", &band.r, &band.g, &band.b); // NOLINT(cert-err34-c)
            if (n != 3) return false;
        }
    } else {
        return false;
    }

	return false;
}
