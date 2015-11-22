#ifndef _SIMULATEDSIMIAN_UTILS_H_INCLUDED__
#define _SIMULATEDSIMIAN_UTILS_H_INCLUDED__

#include <EEPROM.h>

template <typename type>
struct vec3d {
  type x;
  type y;
  type z;
};

template <typename val_t, typename stream_t>
void printVec3d(const val_t& val, stream_t& str) {
  str.print("[");
  str.print(val.x);
  str.print(",");
  str.print(val.y);
  str.print(",");
  str.print(val.z);
  str.print("]");
}

template <typename v_t>
void clearVector(v_t& v) {
  v.x = 0;
  v.y = 0;
  v.z = 0;
}

template <typename v1_t, typename v2_t>
void copyVector(const v1_t& from, v2_t& to) {
  to.x = from.x;
  to.y = from.y;
  to.z = from.z;
}

template <typename v1_t, typename v2_t>
void addVector(const v1_t& from, v2_t& to) {
  to.x += from.x;
  to.y += from.y;
  to.z += from.z;
}

template <typename v1_t, typename scale_t>
v1_t divVector(v1_t v, scale_t scale) {
  v.x /= scale;
  v.y /= scale;
  v.z /= scale;
  return v;
}

template <typename v1_t, typename scale_t>
v1_t mulVector(v1_t v, scale_t scale) {
  v.x *= scale;
  v.y *= scale;
  v.z *= scale;
  return v;
}

template <typename type, size_t sz>
class RunningAverage {
 private:
  type m_data[sz];
  size_t m_pos;

 public:
  RunningAverage() : m_pos(0) { memset(m_data, 0, sizeof(type) * sz); }

  void operator<<(type v) {
    m_pos = (m_pos + 1) % sz;
    m_data[m_pos] = v;
  }

  type val() {
    int32_t v = 0;
    for (size_t n = 0; n < sz; n++) {
      v += m_data[n];
    }
    return v / sz;
  }
};

template <typename type>
int writeStructEEPROM(const type& s, int addr) {
  for (int n = 0; n < sizeof(type); n++) {
    int8_t val = ((int8_t*)&s)[n];
    EEPROM.write(addr + n, val);
    if (val != EEPROM.read(addr + n)) {
      return -1;
    }
  }
  return sizeof(type);
}

template <typename type>
int readStructEEPROM(type& s, int addr) {
  for (int n = 0; n < sizeof(type); n++) {
    ((int8_t*)&s)[n] = EEPROM.read(addr + n);
  }
  return sizeof(type);
}

#endif