// Copyright 2016 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef FPLBASE_PREPROCESSOR_H
#define FPLBASE_PREPROCESSOR_H

#include <set>
#include "fplbase/utilities.h"

namespace fplbase {
/// @brief Load a file like `LoadFile()`, but scan for directives.
///
/// The only supported directives is \#include.
/// @param[in] filename A UTF-8 C-string representing the file to load.
/// @param[out] dest A pointer to a `std::string` to capture the preprocessed
/// version of the file.
/// @param[out] error_message A pointer to a `std::string` that captures an
/// error message (if the function returned `false`, indicating failure).
/// @return If this function returns false, `error_message` indicates which
/// directive caused the problem and why.
bool LoadFileWithDirectives(const char *filename, std::string *dest,
                            std::string *error_message);

/// @brief Overloaded LoadFileWithDirectives to allow pre-definining \#define
/// identifiers.
///
/// @param[in] filename A UTF-8 C-string representing the file to load.
/// @param[out] dest A pointer to a `std::string` to capture the preprocessed
/// version of the file.
/// @param[in] defines A set of identifiers which will be
/// prefixed with \#define at the start of the file.
/// before loading the file.
/// @param[out] error_message A pointer to a `std::string` that captures an
/// error message (if the function returned `false`, indicating failure).
/// @return If this function returns false, `error_message` indicates which
/// directive caused the problem and why.
bool LoadFileWithDirectives(const char *filename, std::string *dest,
                            const std::set<std::string> &defines,
                            std::string *error_message);

/// @brief Overloaded LoadFileWithDirectives to allow pre-definining \#define
/// identifiers in an array.
///
/// @param[in] filename A UTF-8 C-string representing the file to load.
/// @param[out] dest A pointer to a `std::string` to capture the preprocessed
/// version of the file.
/// @param[in] defines A nullptr-terminated array of identifiers which will be
/// prefixed with \#define at the start of the file.
/// before loading the file.
/// @param[out] error_message A pointer to a `std::string` that captures an
/// error message (if the function returned `false`, indicating failure).
/// @return If this function returns false, `error_message` indicates which
/// directive caused the problem and why.
bool LoadFileWithDirectives(const char *filename, std::string *dest,
                            const char * const *defines,
                            std::string *error_message);

/// @brief Prepares OpenGL shaders for compilation across desktop or mobile.
///
/// This function adds platform-specific definitions that allow the same
/// shaders to be used on both desktop and mobile versions of GL.  It does so
/// in a way that is aware of #version and #extension directives, which must
/// be placed before any shader code.
///
/// @param[in] source Shader source to be sanitized.
/// @param[in] defines A nullptr-terminated array of identifiers which will be
/// safely prefixed with \#define at the start of the file.  Can be null.
/// @param[out] result Sanitized source code.
void PlatformSanitizeShaderSource(const char *source,
  const char * const *defines, std::string *result);

}  // namespace fplbase

#endif  // FPLBASE_PREPROCESSOR_H
