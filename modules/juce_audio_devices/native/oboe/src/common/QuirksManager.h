/*
 * Copyright 2019 The Android Open Source Project
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

#ifndef OBOE_QUIRKS_MANAGER_H
#define OBOE_QUIRKS_MANAGER_H

#include <memory>
#include <oboe/AudioStreamBuilder.h>

namespace oboe {

/**
 * Based on manufacturer, model and Android version number
 * decide whether data conversion needs to occur.
 *
 * This also manages device and version specific workarounds.
 */

class QuirksManager {
public:

    static QuirksManager &getInstance() {
        static QuirksManager instance;
        return instance;
    }

    /**
     *
     * @param builder builder provided by application
     * @param childBuilder modified builder appropriate for the underlying device
     * @return true if conversion is needed
     */
    bool isConversionNeeded(const AudioStreamBuilder &builder, AudioStreamBuilder &childBuilder);

private:
};

}
#endif //OBOE_QUIRKS_MANAGER_H
