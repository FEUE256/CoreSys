// Advanced Math Engine

#include <stdint.h>

/*
    AME - Math to C
    CoreSys Copyright 2026 FÈUE License
    
    ABI: int64_t for arguments and return type
         Only 3 decimals (if you fork you can change it)
         Use AME_SCALE for result to work with %a (kprintf)
*/

// Standard AME Pre-Processor macro defines
#define AME_PI3    3142LL
#define AME_PI2    1571LL
#define AME_2PI    6283LL
#define AME_SCALE  1000LL
#define AME_180    180LL

#define AME_PI               "3.1415926536"
#define AME_SQRT3            "1.7320508076"
#define AME_PHI              "1.6180339887"
#define AME_APERY            "1.2020569032"
#define AME_TAU              "6.2831853072"
#define AME_LN2              "0.6931471806"
#define AME_E                "2.7182818285"
#define AME_SQRT2            "1.4142135624"
#define AME_EULER_MASCHERONI "0.5772156649"
#define AME_CATALAN          "0.9159655941"
#define AME_SQRT5            "2.2360679775"
#define AME_KHINCHIN         "2.6854520011"
#define AME_CONWAY           "1.3035772690"
#define AME_SQRT7            "2.6457513111"
#define AME_SQRT10           "3.1622776602"
#define AME_SQRTPI           "1.7724538509"
#define AME_LOG10_2          "0.3010299957"
#define AME_LOG10_E          "0.4342944819"
#define AME_LN10             "2.3025850930"
#define AME_LN3              "1.0986122887"
#define AME_LN5              "1.6094379124"
#define AME_LNPI             "1.1447298858"
#define AME_SQRT_E           "1.6487212707"
#define AME_INV_PI           "0.3183098862"
#define AME_INV_E            "0.3678794412"
#define AME_2_PI             "0.6366197724"
#define AME_PI_SQRT          "1.7724538509"
#define AME_SQRT2_PI         "0.7978845608"
#define AME_GAUSS            "0.8346268417"
#define AME_FEIGENBAUM_DELTA "4.6692016091"
#define AME_FEIGENBAUM_ALPHA "2.5029078751"
#define AME_SILVER_RATIO     "2.4142135624"
#define AME_PLASTIC_NUMBER   "1.3247179572"
#define AME_SUPER_GOLDEN     "1.4655712319"
#define AME_BRUN             "1.9021605831"
#define AME_TWIN_PRIME       "1.9021605831"
#define AME_GOMPERTZ         "0.5963473623"
#define AME_OMEGA            "0.5671432904"
#define AME_RECIPROCAL_PHI   "0.6180339887"
#define AME_SQRT6            "2.4494897428"
#define AME_SQRT8            "2.8284271247"
#define AME_SQRT11           "3.3166247904"
#define AME_SQRT13           "3.6055512755"
#define AME_SQRT17           "4.1231056256"
#define AME_SQRT19           "4.3588989435"
#define AME_SQRT23           "4.7958315233"
#define AME_SQRT29           "5.3851648071"
#define AME_SQRT31           "5.5677643628"

// Standard Forward Defintions
uint64_t k_trnd();

// Basic Math Operations

// Add
int64_t ame_add(int64_t x, int64_t y)
{
    return (x + y) * AME_SCALE;
}

// Subtract
int64_t ame_sub(int64_t x, int64_t y)
{
    return (x - y) * AME_SCALE;
}

// Multiply
int64_t ame_mul(int64_t x, int64_t y)
{
    return (x * y) * AME_SCALE;
}

// Divide
int64_t ame_div(int64_t x, int64_t y)
{
    if (y == 0)
        return INT64_MAX;

    return (x * AME_SCALE) / y;
}

// Modulus
int64_t ame_mod(int64_t x, int64_t y)
{
    if (y == 0)
        return INT64_MAX;

    return (x % y) * AME_SCALE;
}

// Absolute Value
int64_t ame_abs(int64_t x)
{
    return (x < 0 ? -x : x) * AME_SCALE;
}

// Negate
int64_t ame_neg(int64_t x)
{
    return -(x < 0 ? -x : x) * AME_SCALE;
}

// Powers and roots

// Power
int64_t ame_pow(int64_t x, int64_t y)
{
    int64_t result = 1;

    for (int64_t i = 0; i < y; i++)
        result *= x;

    return result * AME_SCALE;
}

// Root
int64_t ame_xqrt(int64_t type, int64_t x)
{
    if (type <= 0 || x < 0)
        return -1;

    if (type == 1)
        return x;

    // Binary search in scaled space
    // We want result r such that (r/SCALE)^type == x/SCALE
    int64_t low    = 0;
    int64_t high   = x;
    int64_t result = 0;

    while (low <= high)
    {
        int64_t mid   = low + (high - low) / 2;
        int64_t power = AME_SCALE;

        for (int64_t i = 0; i < type; i++)
            power = (power * mid) / AME_SCALE;

        if (power <= x)
        {
            result = mid;
            low    = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    return result;
}

// Hypot
int64_t ame_hypot(int64_t x, int64_t y)
{
    int64_t xs = x * AME_SCALE;
    int64_t ys = y * AME_SCALE;

    return ame_xqrt(2, (xs * xs) / AME_SCALE + (ys * ys) / AME_SCALE);
}

// Triangular math

// Sin
int64_t ame_sin(int64_t degrees)
{
    // Lookup table for 85-90 where Taylor loses precision
    // Values: sin(85)=0.996, sin(86)=0.998, sin(87)=0.999,
    //         sin(88)=0.999, sin(89)=1.000, sin(90)=1.000
    if (degrees >= 85 && degrees <= 90)
    {
        static const int64_t lut[] = { 996, 998, 999, 999, 1000, 1000 };
        return lut[degrees - 85];
    }

    if (degrees <= -85 && degrees >= -90)
    {
        static const int64_t lut[] = { -996, -998, -999, -999, -1000, -1000 };
        return lut[-degrees - 85];
    }

    int64_t x = (degrees * AME_PI3) / AME_180;

    x %= AME_2PI;

    if (x > AME_PI3)
        x -= AME_2PI;

    if (x < -AME_PI3)
        x += AME_2PI;

    if (x > AME_PI2)
        x = AME_PI3 - x;

    if (x < -AME_PI2)
        x = -AME_PI3 - x;

    int64_t x2 = (x * x) / AME_SCALE;

    int64_t result = x;

    result -= ((x * x2) / AME_SCALE) / 6;
    result += (((x * x2 / AME_SCALE) * x2) / AME_SCALE) / 120;
    result -= (((((x * x2 / AME_SCALE) * x2) / AME_SCALE) * x2) / AME_SCALE) / 5040;

    if (result >  AME_SCALE) result =  AME_SCALE;
    if (result < -AME_SCALE) result = -AME_SCALE;

    return result;
}

// Cos
int64_t ame_cos(int64_t degrees)
{
    // degrees -> fixed-point radians (*1000)
    int64_t x = degrees * AME_PI3 / AME_180;

    // Normalize to -pi ... pi
    x %= AME_2PI;

    if (x > AME_PI3)
        x -= AME_2PI;

    if (x < -AME_PI3)
        x += AME_2PI;

    // Taylor: cos(x) = 1 - x²/2! + x⁴/4! - x⁶/6!
    int64_t x2 = (x * x) / AME_SCALE;

    int64_t result = AME_SCALE;

    result -= x2 / 2;

    int64_t x4 = (x2 * x2) / AME_SCALE;
    result += x4 / 24;

    int64_t x6 = (x4 * x2) / AME_SCALE;
    result -= x6 / 720;

    if (result > -2 && result < 2)
        result = 0;

    return result;
}

// Tan
int64_t ame_tan(int64_t degrees)
{
    int64_t c = ame_cos(degrees);

    if (c == 0)
        return INT64_MAX; // undefined (90, 270, etc.)

    return (ame_sin(degrees) * AME_SCALE) / c;
}

// Asin
int64_t ame_asin(int64_t value)
{
    if (value >  AME_SCALE) value =  AME_SCALE;
    if (value < -AME_SCALE) value = -AME_SCALE;

    int64_t result = -90;

    for (int64_t deg = -90; deg <= 90; deg++)
    {
        if (ame_sin(deg) <= value)
            result = deg;
    }

    return result * AME_SCALE;
}

// Acos
int64_t ame_acos(int64_t value)
{
    // acos(x) = 90 - asin(x)
    return (90 * AME_SCALE) - ame_asin(value);
}

// Atan
int64_t ame_atan(int64_t value)
{
    int64_t low  = -90;
    int64_t high =  90;

    while (low <= high)
    {
        int64_t mid = (low + high) / 2;
        int64_t t   = ame_tan(mid);

        if (t == value)
            return mid * AME_SCALE;

        if (t < value)
            low = mid + 1;
        else
            high = mid - 1;
    }

    return low * AME_SCALE;
}

// Atan2
int64_t ame_atan2(int64_t y, int64_t x)
{
    if (x > 0)
        return ame_atan((y * AME_SCALE) / x);

    if (x < 0 && y >= 0)
        return ame_atan((y * AME_SCALE) / x) + (180 * AME_SCALE);

    if (x < 0 && y < 0)
        return ame_atan((y * AME_SCALE) / x) - (180 * AME_SCALE);

    if (x == 0 && y > 0)
        return 90 * AME_SCALE;

    if (x == 0 && y < 0)
        return -90 * AME_SCALE;

    return 0;
}

// Exp
int64_t ame_exp(int64_t x)
{
    // e^x = 1+x+x²/2+x³/6+x⁴/24...
    int64_t result = AME_SCALE;
    int64_t term   = AME_SCALE;

    term = (term * x) / AME_SCALE;
    result += term;

    term = (term * x) / AME_SCALE;
    result += term / 2;

    term = (term * x) / AME_SCALE;
    result += term / 6;

    term = (term * x) / AME_SCALE;
    result += term / 24;

    term = (term * x) / AME_SCALE;
    result += term / 120;

    return result;
}

// Sinh
int64_t ame_sinh(int64_t x)
{
    int64_t ex  = ame_exp(x);
    int64_t enx = ame_exp(-x);

    return (ex - enx) / 2;
}

// Cosh
int64_t ame_cosh(int64_t x)
{
    int64_t ex  = ame_exp(x);
    int64_t enx = ame_exp(-x);

    return (ex + enx) / 2;
}

// Tanh
int64_t ame_tanh(int64_t x)
{
    int64_t s = ame_sinh(x);
    int64_t c = ame_cosh(x);

    if (c == 0)
        return INT64_MAX;

    return (s * AME_SCALE) / c;
}

// Asinh
int64_t ame_asinh(int64_t x)
{
    // asinh(x) = ln(x + sqrt(x²+1))
    int64_t x2    = (x * x) / AME_SCALE;
    int64_t value = x2 + AME_SCALE;

    // sqrt approximation
    int64_t root = value;

    for (int i = 0; i < 20; i++)
        root = (root + (value * AME_SCALE) / root) / 2;

    int64_t y  = x + root;
    int64_t z  = ((y - AME_SCALE) * AME_SCALE) / (y + AME_SCALE);
    int64_t z2 = (z * z) / AME_SCALE;

    return 2 * (
        z
        + z2 * z / (3 * AME_SCALE)
        + z2 * z2 * z / (5 * AME_SCALE * AME_SCALE)
    );
}

// Acosh
int64_t ame_acosh(int64_t x)
{
    // acosh(x) = ln(x + sqrt(x²-1))
    if (x < AME_SCALE)
        return 0;

    int64_t x2    = (x * x) / AME_SCALE;
    int64_t value = x2 - AME_SCALE;

    if (value == 0)
        return 0; // acosh(1) = 0

    int64_t root = value;

    for (int i = 0; i < 20; i++)
        root = (root + (value * AME_SCALE) / root) / 2;

    int64_t y  = x + root;
    int64_t z  = ((y - AME_SCALE) * AME_SCALE) / (y + AME_SCALE);
    int64_t z2 = (z * z) / AME_SCALE;

    return 2 * (
        z
        + z2 * z / (3 * AME_SCALE)
        + z2 * z2 * z / (5 * AME_SCALE * AME_SCALE)
    );
}

// Atanh
int64_t ame_atanh(int64_t x)
{
    // atanh(x) = 0.5 * ln((1+x)/(1-x))
    if (x >= AME_SCALE || x <= -AME_SCALE)
        return INT64_MAX;

    int64_t numerator   = AME_SCALE + x;
    int64_t denominator = AME_SCALE - x;
    int64_t y           = (numerator * AME_SCALE) / denominator;
    int64_t z           = ((y - AME_SCALE) * AME_SCALE) / (y + AME_SCALE);
    int64_t z2          = (z * z) / AME_SCALE;

    return z
        + z2 * z / (3 * AME_SCALE)
        + z2 * z2 * z / (5 * AME_SCALE * AME_SCALE);
}

// Log

// Log
int64_t ame_log(int64_t base, int64_t x)
{
    if (base <= 1 || x <= 0)
        return -1;

    // Natural log: base == 0 is the sentinel
    // Caller passes base=0 to mean ln; we use the ln series directly
    if (base == 0)
    {
        // ln(x) via identity ln(x) = 2*atanh((x-1)/(x+1)), x in scaled form
        int64_t xs = x * AME_SCALE;
        int64_t z  = ((xs - AME_SCALE) * AME_SCALE) / (xs + AME_SCALE);
        int64_t z2 = (z * z) / AME_SCALE;

        return 2 * (
            z
            + z2 * z / (3 * AME_SCALE)
            + z2 * z2 * z / (5 * AME_SCALE * AME_SCALE)
            + z2 * z2 * z2 * z / (7 * AME_SCALE * AME_SCALE * AME_SCALE)
        );
    }

    int64_t integer = 0;

    // Integer part
    while (x >= base)
    {
        x /= base;
        integer++;
    }

    int64_t result = integer * AME_SCALE;

    // No fractional part if x reduced to exactly 1
    if (x == 1)
        return result;

    int64_t place = 100;

    while (place > 0)
    {
        x *= base;

        if (x >= base)
        {
            result += place;
            x /= base;
        }

        place /= 10;
    }

    return result;
}

// Rounding

// Floor
int64_t ame_floor(int64_t x)
{
    if (x >= 0)
        return (x / AME_SCALE) * AME_SCALE;
    else
        return ((x - AME_SCALE + 1) / AME_SCALE) * AME_SCALE;
}

// Ceil
int64_t ame_ceil(int64_t x)
{
    if (x >= 0)
        return ((x + AME_SCALE - 1) / AME_SCALE) * AME_SCALE;
    else
        return (x / AME_SCALE) * AME_SCALE;
}

// Round
int64_t ame_round(int64_t x)
{
    if (x >= 0)
        return ((x + AME_SCALE / 2) / AME_SCALE) * AME_SCALE;
    else
        return ((x - AME_SCALE / 2) / AME_SCALE) * AME_SCALE;
}

// Trunc
int64_t ame_trunc(int64_t x)
{
    return (x / AME_SCALE) * AME_SCALE;
}

// Min
int64_t ame_min(int64_t x, int64_t y)
{
    return (x < y ? x : y) * AME_SCALE;
}

// Max
int64_t ame_max(int64_t x, int64_t y)
{
    return (x > y ? x : y) * AME_SCALE;
}

// Signs

// Sign
int64_t ame_sign(int64_t x)
{
    if (x > 0)
        return  1 * AME_SCALE;

    if (x < 0)
        return -1 * AME_SCALE;

    return 0;
}

// Copy Sign
int64_t ame_copysign(int64_t x, int64_t y)
{
    int64_t ax = (x < 0 ? -x : x);

    return (y < 0 ? -ax : ax) * AME_SCALE;
}

// Statistics

// Sum
int64_t ame_sum(int64_t *x, int64_t n)
{
    int64_t sum = 0;

    for (int64_t i = 0; i < n; i++)
        sum += x[i];

    return sum * AME_SCALE;
}

// Average
int64_t ame_average(int64_t *x, int64_t n)
{
    if (n <= 0)
        return 0;

    int64_t sum = 0;

    for (int64_t i = 0; i < n; i++)
        sum += x[i];

    return (sum * AME_SCALE) / n;
}

// Median
int64_t ame_median(int64_t *x, int64_t n)
{
    if (n <= 0)
        return 0;

    for (int64_t i = 0; i < n - 1; i++)
    {
        for (int64_t j = i + 1; j < n; j++)
        {
            if (x[j] < x[i])
            {
                int64_t tmp = x[i];
                x[i] = x[j];
                x[j] = tmp;
            }
        }
    }

    if (n % 2)
        return x[n / 2] * AME_SCALE;

    return ((x[n / 2 - 1] + x[n / 2]) * AME_SCALE) / 2;
}

// Variance
int64_t ame_variance(int64_t *x, int64_t n)
{
    if (n <= 0)
        return 0;

    int64_t mean     = ame_average(x, n);
    int64_t variance = 0;

    for (int64_t i = 0; i < n; i++)
    {
        int64_t d = (x[i] * AME_SCALE) - mean;
        variance += (d * d) / AME_SCALE;
    }

    return (variance / n);
}

// Stddev
int64_t ame_stddev(int64_t *x, int64_t n)
{
    if (n <= 0)
        return 0;

    return ame_xqrt(2, ame_variance(x, n));
}

// Combinatorics

// Factorial
int64_t ame_factorial(int64_t n)
{
    if (n < 0)
        return -1;

    int64_t result = 1;

    for (int64_t i = 2; i <= n; i++)
    {
        // Overflow protection
        if (result > INT64_MAX / i)
            return INT64_MAX;

        result *= i;
    }

    return result * AME_SCALE;
}

// Triangular
int64_t ame_triangular(int64_t n)
{
    if (n < 0)
        return -1;

    int64_t result = 1;

    for (int64_t i = 2; i <= n; i++)
    {
        // Overflow protection
        if (result > INT64_MAX / i)
            return INT64_MAX;

        result += i;
    }

    return result * AME_SCALE;
}

// Gcd
int64_t ame_gcd(int64_t a, int64_t b)
{
    if (a < 0) a = -a;
    if (b < 0) b = -b;

    while (b != 0)
    {
        int64_t temp = b;
        b = a % b;
        a = temp;
    }

    return a * AME_SCALE;
}

// Lcm
int64_t ame_lcm(int64_t a, int64_t b)
{
    if (a == 0 || b == 0)
        return 0;

    int64_t gcd = ame_gcd(a, b) / AME_SCALE;

    int64_t abs_a = (a < 0 ? -a : a);
    int64_t abs_b = (b < 0 ? -b : b);

    // Overflow protection
    if (abs_a / gcd > INT64_MAX / abs_b)
        return INT64_MAX;

    return (abs_a / gcd * abs_b) * AME_SCALE;
}

// nPr
int64_t ame_nPr(int64_t n, int64_t r)
{
    if (n < 0 || r < 0 || r > n)
        return -1;

    int64_t result = 1;

    for (int64_t i = 0; i < r; i++)
    {
        int64_t value = n - i;

        if (result > INT64_MAX / value)
            return INT64_MAX;

        result *= value;
    }

    return result * AME_SCALE;
}

// nCr
int64_t ame_nCr(int64_t n, int64_t r)
{
    if (n < 0 || r < 0 || r > n)
        return -1;

    if (r > n - r)
        r = n - r;

    int64_t result = 1;

    for (int64_t i = 1; i <= r; i++)
    {
        int64_t numerator = n - r + i;

        /*
         * Divide before multiplying when possible.
         *
         * This keeps intermediate values smaller than
         * calculating n! directly.
         */
        int64_t g = ame_gcd(numerator, i) / AME_SCALE;

        numerator       /= g;
        int64_t denominator = i / g;

        if (result > INT64_MAX / numerator)
            return INT64_MAX;

        result *= numerator;
        result /= denominator;
    }

    return result * AME_SCALE;
}

// Number Theory

// Isprime
int64_t ame_isPrime(int64_t n)
{
    if (n < 2)
        return 0;

    if (n == 2)
        return AME_SCALE;

    if (n % 2 == 0)
        return 0;

    for (int64_t i = 3; i <= n / i; i += 2)
    {
        if (n % i == 0)
            return 0;
    }

    return AME_SCALE;
}

// Primefactors
void ame_primeFactors(int64_t n)
{
    if (n < 0)
        n = -n;

    if (n < 2)
    {
        kprintf("none");
        return;
    }

    bool first = true;

    while (n % 2 == 0)
    {
        if (!first)
            kprintf(" * ");

        kprintf("%d", 2);

        first = false;
        n /= 2;
    }

    for (int64_t i = 3; i <= n / i; i += 2)
    {
        while (n % i == 0)
        {
            if (!first)
                kprintf(" * ");

            kprintf("%d", i);

            first = false;
            n /= i;
        }
    }

    if (n > 1)
    {
        if (!first)
            kprintf(" * ");

        kprintf("%d", n);
    }
}

// Fibonacci
int64_t ame_fibonacci(int64_t n)
{
    if (n < 0)
        return -AME_SCALE;

    if (n == 0)
        return 0;

    if (n == 1)
        return AME_SCALE;

    int64_t a = 0;
    int64_t b = AME_SCALE;

    for (int64_t i = 2; i <= n; i++)
    {
        int64_t next = a + b;

        /* Basic overflow protection */
        if (next < b)
            return INT64_MAX;

        a = b;
        b = next;
    }

    return b;
}

// Iseven
int64_t ame_isEven(int64_t n)
{
    return ((n % 2) == 0)
        ? AME_SCALE
        : 0;
}

// Isodd
int64_t ame_isOdd(int64_t n)
{
    return ((n % 2) != 0)
        ? AME_SCALE
        : 0;
}

// Random numbers

// Random
int64_t ame_random(void)
{
    return (int64_t)(k_trnd() & INT64_MAX);
}

// RandomRange
int64_t ame_randomRange(int64_t min, int64_t max)
{
    if (min > max)
    {
        int64_t temp = min;
        min = max;
        max = temp;
    }

    if (min == max)
        return min * AME_SCALE;

    uint64_t range =
        (uint64_t)(max - min + 1);

    uint64_t value =
        k_trnd();

    return (int64_t)(
        min + (value % range)
    ) * AME_SCALE;
}

// RandomNormal
int64_t ame_randomNormal(void)
{
    int64_t sum = 0;

    for (int i = 0; i < 12; i++)
    {
        uint64_t value = k_trnd();

        /*
         * Convert random value to approximately
         * uniform [0,1].
         */
        sum += (int64_t)(value % AME_SCALE);
    }

    /*
     * Sum of 12 U(0,1) variables has:
     * mean = 6
     * variance = 1
     *
     * Subtract 6 to center around zero.
     */
    return sum - (6 * AME_SCALE);
}

// Noise
int64_t ame_noise(int64_t x)
{
    uint64_t n = (uint64_t)x;

    n ^= n >> 33;
    n *= 0xff51afd7ed558ccdULL;
    n ^= n >> 33;
    n *= 0xc4ceb9fe1a85ec53ULL;
    n ^= n >> 33;

    return (int64_t)(
        n % (2 * AME_SCALE)
    ) - AME_SCALE;
}
