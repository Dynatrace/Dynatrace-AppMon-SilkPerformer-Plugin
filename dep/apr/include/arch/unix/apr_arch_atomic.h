/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ATOMIC_H
#define ATOMIC_H

#include "apr.h"
#include "apr_private.h"
#include "apr_atomic.h"
#include "apr_thread_mutex.h"

#if defined(USE_ATOMICS_GENERIC)
/* noop */
#warning USE_ATOMICS_GENERIC
#elif defined(__GNUC__) && defined(__STRICT_ANSI__)
/* force use of generic atomics if building e.g. with -std=c89, which
 * doesn't allow inline asm */
#   define USE_ATOMICS_GENERIC
#warning USE_ATOMICS_GENERIC
#elif defined(SOLARIS2) && SOLARIS2 >= 10
#   define USE_ATOMICS_SOLARIS10
#warning USE_ATOMICS_SOLARIS10
/* __sparc_v9__ only defined by gcc */
#elif defined(__sparc) && defined(__sparc_v9__)
#   define USE_ATOMICS_SPARCV9 
#warning USE_ATOMICS_SPARCV9
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#   define USE_ATOMICS_IA32
#warning USE_ATOMICS_IA32
#elif defined(__GNUC__) && (defined(__PPC__) || defined(__ppc__) || defined(_ARCH_PPC) || defined(_M_PPC))
#   define USE_ATOMICS_PPC
#warning USE_ATOMICS_PPC
#elif defined(__GNUC__) && (defined(__s390__) || defined(__s390x__))
#   define USE_ATOMICS_S390
#warning USE_ATOMICS_S390
#elif HAVE_ATOMIC_BUILTINS
#   define USE_ATOMICS_BUILTINS
#warning USE_ATOMICS_BUILTINS
#else
#   define USE_ATOMICS_GENERIC
#warning USE_ATOMICS_GENERIC
#endif

#endif /* ATOMIC_H */
