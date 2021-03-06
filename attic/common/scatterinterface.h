/*  This file is part of the Vc library. {{{
Copyright © 2014-2017 Matthias Kretz <kretz@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the names of contributing organizations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

}}}*/

///////////////////////////////////////////////////////////////////////////////////////////
// scatters
// A scatter takes the following arguments:
// 1. A pointer to memory of any type that EntryType can convert to.
// 2. An indexes “vector”. The requirement is that the type implements the subscript operator,
//    stores «Size» valid index values, and each offset to the pointer above yields a valid
//    memory location for reading.
// 3. Optionally the third argument may be a mask. The mask disables several memory stores and
//    thus removes the requirements in (2.) for the disabled entries.

private:
    /**\internal
     * This function implements a scatter given a pointer to memory \p mem and some
     * container object storing the scatter \p indexes.
     *
     * \param mem This pointer must be aligned correctly for the type \p MT. This is the
     * natural behavior of C++, so this is typically the case.
     * \param indexes This object contains at least \VSize{T} indexes that denote the
     * offset in \p mem where the components for the current vector should be copied to.
     * The offset is not in Bytes, but in multiples of `sizeof(MT)`.
     */
    // enable_if<std::can_convert<MT, EntryType>::value && has_subscript_operator<IT>::value>
    template <typename MT, typename IT>
    inline void scatterImplementation(MT *mem, IT &&indexes) const;

    /**\internal
     * This overload of the above function adds a \p mask argument to disable memory
     * accesses at the \p indexes offsets where \p mask is \c false.
     */
    template <typename MT, typename IT>
    inline void scatterImplementation(MT *mem, IT &&indexes, MaskArgument mask) const;

public:
#define Vc_ASSERT_SCATTER_PARAMETER_TYPES_                                               \
    static_assert(                                                                       \
        std::is_convertible<EntryType, MT>::value,                                       \
        "The memory pointer needs to point to a type that the EntryType of this "        \
        "SIMD vector type can be converted to.");                                        \
    static_assert(                                                                       \
        Vc::Traits::has_subscript_operator<IT>::value,                                   \
        "The indexes argument must be a type that implements the subscript operator.");  \
    static_assert(                                                                       \
        !Traits::is_simd_vector<IT>::value ||                                            \
            Traits::simd_vector_size<IT>::value >= Size,                                 \
        "If you use a SIMD vector for the indexes parameter, the index vector must "     \
        "have at least as many entries as this SIMD vector.");                           \
    static_assert(                                                                       \
        !std::is_array<T>::value ||                                                      \
            (std::rank<T>::value == 1 &&                                                 \
             (std::extent<T>::value == 0 || std::extent<T>::value >= Size)),             \
        "If you use a simple array for the indexes parameter, the array must have "      \
        "at least as many entries as this SIMD vector.")

    /**
     * \name Scatter functions
     *
     * Stores a vector to the objects at `mem[indexes[0]]`, `mem[indexes[1]]`,
     * `mem[indexes[2]]`, ...
     *
     * \param mem A pointer to memory which contains objects of type \p MT at the offsets
     *            given by \p indexes.
     * \param indexes
     * \param mask
     */
    ///@{

    /// Scatter function
    template <typename MT,
              typename IT,
              typename = enable_if<Vc::Traits::has_subscript_operator<IT>::value>>
    Vc_INTRINSIC void scatter(MT *mem, IT &&indexes) const
    {
        Vc_ASSERT_SCATTER_PARAMETER_TYPES_;
        scatterImplementation(mem, std::forward<IT>(indexes));
    }

    /// Masked scatter function
    template <typename MT,
              typename IT,
              typename = enable_if<Vc::Traits::has_subscript_operator<IT>::value>>
    Vc_INTRINSIC void scatter(MT *mem, IT &&indexes, MaskArgument mask) const
    {
        Vc_ASSERT_SCATTER_PARAMETER_TYPES_;
        scatterImplementation(mem, std::forward<IT>(indexes), mask);
    }
    ///@}

    /// \name Deprecated Members
    ///@{

    /**
     * \deprecated Use Vc::array or Vc::vector subscripting instead.
     *
     * \param array   A pointer into memory (without alignment restrictions).
     * \param member1 If \p array points to a struct, \p member1 determines the member in the struct to
     *                be read. Thus the offsets in \p indexes are relative to the \p array and not to
     *                the size of the gathered type (i.e. array[i].*member1 is accessed instead of
     *                (&(array->*member1))[i])
     * \param indexes Determines the offsets into \p array where the values are gathered from/scattered
     *                to. The type of indexes can either be an integer vector or a type that supports
     *                operator[] access.
     */
    template <typename S1, typename IT>
    Vc_DEPRECATED("use the subscript operator to Vc::array or Vc::vector "
                  "instead.") inline void scatter(S1 *array, EntryType S1::*member1,
                                                  IT indexes) const
    {
        scatter(Common::SubscriptOperation<S1, IT, std::ratio<1, 1>, true>(
                    array, indexes)[member1]
                    .scatterArguments());
    }

    /**
     * \deprecated Use Vc::array or Vc::vector subscripting instead.
     *
     * \param array   A pointer into memory (without alignment restrictions).
     * \param member1 If \p array points to a struct, \p member1 determines the member in the struct to
     *                be read. Thus the offsets in \p indexes are relative to the \p array and not to
     *                the size of the gathered type (i.e. array[i].*member1 is accessed instead of
     *                (&(array->*member1))[i])
     * \param indexes Determines the offsets into \p array where the values are gathered from/scattered
     *                to. The type of indexes can either be an integer vector or a type that supports
     *                operator[] access.
     * \param mask    If a mask is given only the active entries will be gathered/scattered.
     */
    template <typename S1, typename IT>
    Vc_DEPRECATED("use the subscript operator to Vc::array or Vc::vector "
                  "instead.") inline void scatter(S1 *array, EntryType S1::*member1,
                                                  IT indexes, MaskArgument mask) const
    {
        scatter(Common::SubscriptOperation<S1, IT, std::ratio<1, 1>, true>(
                    array, indexes)[member1]
                    .scatterArguments(),
                mask);
    }

    /**
     * \deprecated Use Vc::array or Vc::vector subscripting instead.
     *
     * \param array   A pointer into memory (without alignment restrictions).
     * \param member1 If \p array points to a struct, \p member1 determines the member in the struct to
     *                be read. Thus the offsets in \p indexes are relative to the \p array and not to
     *                the size of the gathered type (i.e. array[i].*member1 is accessed instead of
     *                (&(array->*member1))[i])
     * \param member2 If \p member1 is a struct then \p member2 selects the member to be read from that
     *                struct (i.e. array[i].*member1.*member2 is read).
     * \param indexes Determines the offsets into \p array where the values are gathered from/scattered
     *                to. The type of indexes can either be an integer vector or a type that supports
     *                operator[] access.
     */
    template <typename S1, typename S2, typename IT>
    Vc_DEPRECATED("use the subscript operator to Vc::array or Vc::vector "
                  "instead.") inline void scatter(S1 *array, S2 S1::*member1,
                                                  EntryType S2::*member2,
                                                  IT indexes) const
    {
        scatter(Common::SubscriptOperation<S1, IT, std::ratio<1, 1>, true>(
                    array, indexes)[member1][member2]
                    .scatterArguments());
    }

    /**
     * \deprecated Use Vc::array or Vc::vector subscripting instead.
     *
     * \param array   A pointer into memory (without alignment restrictions).
     * \param member1 If \p array points to a struct, \p member1 determines the member in the struct to
     *                be read. Thus the offsets in \p indexes are relative to the \p array and not to
     *                the size of the gathered type (i.e. array[i].*member1 is accessed instead of
     *                (&(array->*member1))[i])
     * \param member2 If \p member1 is a struct then \p member2 selects the member to be read from that
     *                struct (i.e. array[i].*member1.*member2 is read).
     * \param indexes Determines the offsets into \p array where the values are gathered from/scattered
     *                to. The type of indexes can either be an integer vector or a type that supports
     *                operator[] access.
     * \param mask    If a mask is given only the active entries will be gathered/scattered.
     */
    template <typename S1, typename S2, typename IT>
    Vc_DEPRECATED("use the subscript operator to Vc::array or Vc::vector "
                  "instead.") inline void scatter(S1 *array, S2 S1::*member1,
                                                  EntryType S2::*member2, IT indexes,
                                                  MaskArgument mask) const
    {
        scatter(Common::SubscriptOperation<S1, IT, std::ratio<1, 1>, true>(
                    array, indexes)[member1][member2]
                    .scatterArguments(),
                mask);
    }

    /**
     * \deprecated Use Vc::array or Vc::vector subscripting instead.
     *
     * \param array   A pointer into memory (without alignment restrictions).
     * \param ptrMember1 If \p array points to a struct, \p member1 determines the member in the struct to
     *                be read. Thus the offsets in \p indexes are relative to the \p array and not to
     *                the size of the gathered type (i.e. array[i].*member1 is accessed instead of
     *                (&(array->*member1))[i])
     * \param outerIndexes
     * \param innerIndexes
     */
    template <typename S1, typename IT1, typename IT2>
    Vc_DEPRECATED("use the subscript operator to Vc::array or Vc::vector "
                  "instead.") inline void scatter(S1 *array, EntryType *S1::*ptrMember1,
                                                  IT1 outerIndexes,
                                                  IT2 innerIndexes) const
    {
        scatter(Common::SubscriptOperation<S1, IT1, std::ratio<1, 1>, true>(
                    array, outerIndexes)[ptrMember1][innerIndexes]
                    .scatterArguments());
    }

    /**
     * \deprecated Use Vc::array or Vc::vector subscripting instead.
     *
     * \param array   A pointer into memory (without alignment restrictions).
     * \param ptrMember1 If \p array points to a struct, \p member1 determines the member in the struct to
     *                be read. Thus the offsets in \p indexes are relative to the \p array and not to
     *                the size of the gathered type (i.e. array[i].*member1 is accessed instead of
     *                (&(array->*member1))[i])
     * \param outerIndexes
     * \param innerIndexes
     * \param mask    If a mask is given only the active entries will be gathered/scattered.
     */
    template <typename S1, typename IT1, typename IT2>
    Vc_DEPRECATED("use the subscript operator to Vc::array or Vc::vector "
                  "instead.") inline void scatter(S1 *array, EntryType *S1::*ptrMember1,
                                                  IT1 outerIndexes, IT2 innerIndexes,
                                                  MaskArgument mask) const
    {
        scatter(Common::SubscriptOperation<S1, IT1, std::ratio<1, 1>, true>(
                    array, outerIndexes)[ptrMember1][innerIndexes]
                    .scatterArguments(),
                mask);
    }
    ///@}

    /**\internal
     * \name Scatter function to use from Vc::Common::subscript_operator
     *
     * \param args
     * \param mask
     */
    ///@{
    template <typename MT, typename IT>
    Vc_INTRINSIC void scatter(const Common::ScatterArguments<MT, IT> &args) const
    {
        scatter(args.address, args.indexes);
    }

    template <typename MT, typename IT>
    Vc_INTRINSIC void scatter(const Common::ScatterArguments<MT, IT> &args, MaskArgument mask) const
    {
        scatter(args.address, args.indexes, mask);
    }
    ///@}
#undef Vc_ASSERT_SCATTER_PARAMETER_TYPES_
