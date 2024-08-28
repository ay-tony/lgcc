int main() {
  const float a = 3 - 2 + 7;
  int b = a;
  a = 3.0 * 2;
  float const c = (a + b) / 2;
  return c;
}
