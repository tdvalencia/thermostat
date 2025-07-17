import numpy as np

# === Input your data here ===
# Temperatures in Celsius
temps_c = np.array([24.8, 36, 42.6, 55, 67.8, 75.8, 85.7, 93, 100, 116, 120, 150])

# Resistances in ohms
resistances = np.array([91142, 55000, 41000, 24700, 14500, 12700, 10100, 9000, 7500, 4670, 3670, 2250])

# Convert temperatures to Kelvin
temps_k = temps_c + 273.15

# Take natural log of resistance
ln_r = np.log(resistances)

# Create matrix for 5-term polynomial: [1, ln(R), ln(R)^2, ln(R)^3, ln(R)^4]
X = np.column_stack([
    np.ones_like(ln_r),
    ln_r,
    ln_r**2,
    ln_r**3,
    ln_r**4,
    ln_r**5,
    ln_r**6,
    ln_r**7,
    ln_r**8,
    ln_r**9
])

# 1/T is the target
y = 1 / temps_k

# Solve least squares
coeffs, *_ = np.linalg.lstsq(X, y, rcond=None)

# Print results
terms = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J']
print("Extended Steinhart-Hart Coefficients:")
for term, value in zip(terms, coeffs):
    print(f"{term} = {value:.10e},")
