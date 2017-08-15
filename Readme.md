# SELib

A portable open-source SEAnim and SEModel library for C.

### What's working
 - SEAnim loading
### What's not working
 - SEAnim saving
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