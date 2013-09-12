/* Taken from include/linux/kernel.h from the Linux kernel tree */

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */

#ifndef PIOS_STRUCT_HELPER_H
#define PIOS_STRUCT_HELPER_H

#define container_of(ptr, type, member) \
    ({                      \
         const typeof(((type *)0)->member) * __mptr = (ptr);    \
         (type *)((char *)__mptr - offsetof(type, member)); } \
    )

/**
 * cast_struct_to_array casts an homogeneous structure instance to an array
 * of typeof(struct_field). struct_field need to be any of the fields
 * containing inside the struct
 * @instance: homogeneous structure to cast
 * @struct_field: a field contained inside the structure
 */
#define cast_struct_to_array(instance, struct_field) \
    ((typeof(struct_field) *) & (instance))
#endif /* PIOS_STRUCT_HELPER_H */
