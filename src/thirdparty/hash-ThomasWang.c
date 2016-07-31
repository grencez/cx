
/** 32-bit hash function from Thomas Wang.
 *
 * Found here: http://burtleburtle.net/bob/hash/integer.html
 **/
qual_inline
  uint32_t
uint32_hash_ThomasWang (uint32_t a)
{
  a = (a ^ 61) ^ (a >> 16);
  a = a + (a << 3);
  a = a ^ (a >> 4);
  a = a * 0x27d4eb2d;
  a = a ^ (a >> 15);
  return a;
}


/** 64-bit hash function from Thomas Wang.
 *
 * Found here: https://naml.us/blog/tag/thomas-wang
 **/
qual_inline
  uint64_t
uint64_hash_ThomasWang(uint64_t key)
{
  key = (~key) + (key << 21); // key = (key << 21) - key - 1;
  key = key ^ (key >> 24);
  key = (key + (key << 3)) + (key << 8); // key * 265
  key = key ^ (key >> 14);
  key = (key + (key << 2)) + (key << 4); // key * 21
  key = key ^ (key >> 28);
  key = key + (key << 31);
  return key;
}

