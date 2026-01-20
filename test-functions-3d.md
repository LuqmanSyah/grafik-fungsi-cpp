# Kumpulan Fungsi 3D untuk Testing
## Grafik Fungsi 2 Variabel f(x,y)

Gunakan fungsi-fungsi berikut di input box untuk testing visualisasi grafik 3D.

---

## 1. Basic Surfaces

### Plane
```
x + y
x - y
2*x + 3*y - 1
```

### Paraboloid
```
x^2 + y^2
x^2 - y^2
(x^2 + y^2) / 2
```

### Cone
```
sqrt(x^2 + y^2)
sqrt(x^2 + y^2) - 1
```

### Sphere
```
sqrt(1 - x^2 - y^2)
-sqrt(1 - x^2 - y^2)
```

### Cylinder
```
sqrt(1 - x^2)
-sqrt(1 - x^2)
```

---

## 2. Trigonometric Surfaces

### Wave Functions
```
sin(x) + sin(y)
sin(x) * cos(y)
cos(x) + cos(y)
sin(x) * y
cos(x) * sin(y)
```

### Ripple Effect
```
sin(sqrt(x^2 + y^2))
sin(sqrt(x^2 + y^2) * 2)
cos(sqrt(x^2 + y^2)) / (sqrt(x^2 + y^2) + 1)
```

### Complex Waves
```
sin(x + y) * cos(x - y)
sin(x * y)
sin(x) * sin(y) * 2
(sin(x) + cos(y)) / 2
```

---

## 3. Polynomial Surfaces

### Quadratic
```
x^2 + y^2 - 2
x^2 - y^2 + 1
x*y
2*x^2 + 3*y^2 - x
```

### Cubic
```
x^3 + y^3
x^3 - y^3
x^2*y + y^2*x
(x^2 + y^2) * x
```

### Higher Order
```
x^4 + y^4
(x^2 + y^2)^2
x^5 - y^3
```

---

## 4. Exponential & Logarithmic

### Exponential
```
exp(x) + exp(y)
exp(x^2 + y^2)
exp(-(x^2 + y^2))
exp(-sqrt(x^2 + y^2))
```

### Logarithmic
```
ln(x^2 + y^2 + 1)
log(x^2 + 1) + log(y^2 + 1)
ln(abs(x) + abs(y) + 1)
```

### Gaussian
```
exp(-(x^2 + y^2) / 2)
2 * exp(-(x^2 + y^2))
exp(-((x-1)^2 + (y-1)^2))
```

---

## 5. Saddle & Hyperbolic

### Monkey Saddle
```
x^3 - 3*x*y^2
```

### Classic Saddle
```
x^2 - y^2
(x^2 - y^2) / 2
```

### Hyperbolic
```
sinh(x) * sinh(y)
cosh(x) - cosh(y)
tanh(x) + tanh(y)
```

---

## 6. Peaks & Valleys

### Single Peak
```
exp(-(x^2 + y^2))
1 / (1 + x^2 + y^2)
```

### Multiple Peaks
```
exp(-(x^2 + y^2)) + 0.5*exp(-((x-2)^2 + (y-2)^2))
sin(x) * sin(y) * exp(-(x^2 + y^2) / 5)
```

### Mexican Hat (Ricker Wavelet)
```
(1 - x^2 - y^2) * exp(-(x^2 + y^2) / 2)
```

---

## 7. Spiral & Radial

### Spiral
```
atan(y/x) * sin(x)
atan(y/x) * sin(sqrt(x^2 + y^2))
```

### Radial
```
sin(sqrt(x^2 + y^2))
cos(atan2(y, x) * 3 + sqrt(x^2 + y^2))
sqrt(x^2 + y^2) * sin(atan2(y, x) * 3)
```

---

## 8. Mixed Functions

### Trig + Poly
```
sin(x^2 + y^2)
cos(x*y)
sin(x^2) * cos(y^2)
```

### Exp + Trig
```
exp(-x^2) * sin(y)
exp(-y^2) * cos(x)
exp(-(x^2 + y^2)) * sin(10*x) * cos(10*y)
```

### Log + Trig
```
ln(x^2 + 1) * sin(y)
log(y^2 + 1) * cos(x)
```

---

## 9. Special Surfaces

### Sombrero
```
sin(sqrt(x^2 + y^2)) / (sqrt(x^2 + y^2) + 0.1)
```

### Crater
```
exp(-x^2 - y^2) - exp(-(x^2 + y^2) * 4)
```

### Terrain-like
```
sin(x) * cos(y) + 0.5*sin(2*x) * cos(2*y)
sin(x) + cos(y) + 0.5*sin(2*x) + 0.3*cos(2*y)
```

### Egg Carton
```
sin(x) * cos(y)
sin(x) * sin(y)
cos(x) * cos(y)
```

---

## 10. Absolute Value & Floor

### Absolute
```
abs(x) + abs(y)
abs(x^2 - y^2)
abs(sin(x)) + abs(cos(y))
```

### Floor/Ceil
```
floor(x) + floor(y)
ceil(sin(x) + cos(y))
floor(sqrt(x^2 + y^2))
```

---

## Tips Testing:

1. **Range**: Grafik menampilkan area x dan y dari -3.5 hingga 3.5
2. **Validasi**: Fungsi dengan error akan menampilkan border merah
3. **Navigation**: Gunakan tombol panah untuk mengedit input
4. **History**: Klik item history untuk memuat ulang fungsi sebelumnya
5. **View**: Drag untuk rotate, scroll untuk zoom, R untuk reset view

---

## Catatan:

- Gunakan `^` untuk pangkat (contoh: `x^2` untuk x kuadrat)
- Variabel yang tersedia: `x` dan `y`
- Fungsi matematika: sin, cos, tan, asin, acos, atan, sinh, cosh, tanh, exp, ln, log, sqrt, abs, floor, ceil
- Operator: `+`, `-`, `*`, `/`, `^`
