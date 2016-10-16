typedef double* V3;

static inline void v3_add(V3 a, V3 b, V3 c) {
  c[0] = a[0] + b[0];
  c[1] = a[1] + b[1];
  c[2] = a[2] + b[2];
}

static inline void v3_subtract(V3 a, V3 b, V3 c) {
  c[0] = a[0] - b[0];
  c[1] = a[1] - b[1];
  c[2] = a[2] - b[2];
}

static inline void v3_scale(V3 a, double s, V3 c) {
  c[0] = s * a[0];
  c[1] = s * a[1];
  c[2] = s * a[2];
}

static inline double v3_dot(V3 a, V3 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

static inline void v3_cross(V3 a, V3 b, V3 c) {
  c[0] = a[1]*b[2] - a[2]*b[1];
  c[1] = a[2]*b[0] - a[0]*b[2];
  c[2] = a[0]*b[1] - a[1]*b[0];
}


void example() {
  double* data = {1.0, 2.0, 3.0, 4.0};
  data[0] .. data[1] .. data[2]

    v3_scale(data, 4, 
  

}
