# SELib

A portable open-source SEAnim and SEModel library for C.

SELib is self-contained within a few files that you can easily copy and compile into your application/engine:

 - SELib.h
 - SEAnim.h
 - SEAnim.c
 - SELib_internal.h
 - SELib_internal.c
 - SELib_config.h (empty by default, editable by user)

No specific build process is required to compile SELib. You can compile/include it in any way you want.

### What's working

 - SEAnim loading
 - SEAnim saving

### What's not working

 - SEModel loading/saving (waiting for specification document)

## Examples

### Load a SEAnim

```cpp
    SEAnim_File_t characterIdleAnimation;
    FILE* f;
    f = fopen("character_idle.seanim", "rb");
    int r = LoadSEAnim(&characterIdleAnimation, f);
    if (r == 0)
    {
        printf("SEAnim loaded successfully! It has %u frames.", characterIdleAnimation.header.frameCount);
    }
```
