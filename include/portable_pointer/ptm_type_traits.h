/*
 * Version 1.0
 *
 * Mesut Kuscu - mesut.kuscu@hp.com
 *
 *
 */

#ifndef PTM_TYPE_TRAITS_H
#define PTM_TYPE_TRAITS_H


namespace ptm_type_traits{

    template <typename T>
    struct add_reference
    {
        typedef T& type;
    };
}

#endif //HAT_ON_FUNKY_CHUNKY_PTM_TYPE_TRAITS_H