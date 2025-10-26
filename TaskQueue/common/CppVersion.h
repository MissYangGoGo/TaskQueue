//
//  CppVersion.h
//  videosystem
//
//  Created by SDK Team on 2023/3/2.
//

#ifndef CppVersion_h
#define CppVersion_h

#if defined(__clang__) || defined(__GNUC__)
#define CPP_STANDARD __cplusplus
#elif defined(_MSC_VER)
#define CPP_STANDARD _MSVC_LANG
#endif

#if CPP_STANDARD >= 199711L
#define SUPPORT_CPP03 1
#endif

#if CPP_STANDARD >= 201103L
#define SUPPORT_CPP11 1
#endif

#if CPP_STANDARD >= 201402L
#define SUPPORT_CPP14 1
#endif

#if CPP_STANDARD >= 201703L
#define SUPPORT_CPP17 1
#endif

#endif /* CppVersion_h */
