/**
 * @file oe.h
 * @brief Extenstions to OS functions.
 */

#pragma once

#include "types.h"

namespace nn
{
    namespace oe
    {
        void Initialize();
        void FinishStartupLogo();
        struct DisplayVersion {
            char name[16];
        };
        void GetDisplayVersion(DisplayVersion*);
    };
};