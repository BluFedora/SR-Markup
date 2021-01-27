/******************************************************************************/
/*!
 * @file   bf_array_t.h
 * @author Shareef Abdoul-Raheem
 * @brief
 *   Part of the "BluFedora Data Structures C Library"
 *   This is a generic dynamic array class.
 * 
 *   No dependencies besides the C Standard Library.
 *   Random Access - O(1)
 *   Pop           - O(1)
 *   Push, Emplace - O(1) best O(n) worst (when we need to grow)
 *   Clear         - O(1)
 * 
 *   The memory layout:
 *     [ArrayHeader (capacity | size | stride)][alignment-padding][allocation-offset]^[array-data (uint8_t*)]
 *     The '^' symbol indicates the pointer you are handed back from the 'bfArray_new' function.
 * 
 *   Memory Allocation Customization:
 *     To use your own allocator pass an allocation function to the array.
 *     To be compliant you must:
 *       > Act as malloc  when ptr == NULL, size == number of bytes to alloc.
 *       > Act as free    when ptr != NULL, size == number of bytes given back.
 * 
 * @copyright Copyright (c) 2019-2020
*/
/******************************************************************************/
#ifndef BF_DATA_STRUCTURES_ARRAY_H
#define BF_DATA_STRUCTURES_ARRAY_H

#include <stddef.h> /* size_t */

#if __cplusplus
extern "C" {
#endif
#define k_bfArrayInvalidIndex ((size_t)(-1))

typedef void* (*bfArrayAllocator)(void* user_data, void* ptr, size_t size);

/* a  < b : < 0 */
/* a == b :   0 */
/* a  > b : > 0 */
typedef int(/*__cdecl*/ *bfArraySortCompare)(const void*, const void*);
typedef int(/*__cdecl*/ *bfArrayFindCompare)(const void*, const void*);

#if __cplusplus
#include <type_traits> /* remove_reference_t */
#define bfArray_new(T, alloc, user_data) (T*)bfArray_new_(alloc, sizeof(T), alignof(T), (user_data))
#define bfArray_newA(arr, alloc, user_data) bfArray_new(typename std::remove_reference_t<decltype((arr)[0])>, alloc, user_data)
#else
#define bfArray_new(T, alloc, user_data) (T*)bfArray_new_(alloc, sizeof(T), _Alignof(T), (user_data))
#define bfArray_newA(arr, alloc, user_data) bfArray_new((arr)[0], alloc, user_data)
#endif

// For all functions pass a pointer to the array since some function may reallocate.

void*  bfArray_mallocator(void* user_data, void* ptr, size_t size);                                                         // Default allocator uses C-Libs 'malloc' and 'free'.
void*  bfArray_new_(bfArrayAllocator allocator, size_t element_size, size_t element_alignment, void* allocator_user_data);  // if allocator == NULL then allocator = bfArray_mallocator;
void*  bfArray_userData(void** self);
void*  bfArray_begin(void** self);
void*  bfArray_end(void** self);
void*  bfArray_back(void** self);
size_t bfArray_size(void** self);
size_t bfArray_capacity(void** self);
void   bfArray_copy(void** self, void** src, size_t num_elements);
void   bfArray_clear(void** self);
void   bfArray_reserve(void** self, size_t num_elements);
void   bfArray_resize(void** self, size_t num_elements);
void   bfArray_push(void** self, const void* element);
void   bfArray_insert(void** self, size_t index, const void* element);
void*  bfArray_insertEmplace(void** self, size_t index);
void*  bfArray_emplace(void** self);
void*  bfArray_emplaceN(void** self, size_t num_elements);
void*  bfArray_at(void** self, size_t index);
void*  bfArray_binarySearchRange(void** self, size_t bgn, size_t end, const void* key, bfArrayFindCompare compare); /* [bgn, end) */
void*  bfArray_binarySearch(void** self, const void* key, bfArrayFindCompare compare);
/* If [compare] is NULL then the default impl uses a memcmp with the sizeof a single element. */
/* [key] is always the first parameter for each comparison.                                   */
/* Returns BIFROST_ARRAY_INVALID_INDEX on failure to find element.                            */
size_t bfArray_findInRange(void** self, size_t bgn, size_t end, const void* key, bfArrayFindCompare compare);
size_t bfArray_find(void** self, const void* key, bfArrayFindCompare compare);
void   bfArray_removeAt(void** self, size_t index);
void   bfArray_swapAndPopAt(void** self, size_t index);
void*  bfArray_pop(void** self);
void   bfArray_sortRange(void** self, size_t bgn, size_t end, bfArraySortCompare compare); /* [bgn, end) */
void   bfArray_sort(void** self, bfArraySortCompare compare);
void   bfArray_delete(void** self);
#if __cplusplus
}
#endif

#endif /* BF_DATA_STRUCTURES_ARRAY_H */
