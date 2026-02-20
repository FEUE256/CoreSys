# TR

This document lists the `tc.*` subsystem APIs.

## Core Initialization
- `tc.init();` - Initializes the TC runtime.

## Math / Variable Operations
- `tc.math.assign(name, value);` - Assigns a value to a variable.
- `tc.math.add(a, b);` - Adds two values.
- `tc.math.subtract(a, b);` - Subtracts `b` from `a`.
- `tc.math.multiply(a, b);` - Multiplies two values.
- `tc.math.divide(a, b);` - Divides `a` by `b`.
- `tc.math.modulo(a, b);` - Modulus operation (`a % b`).
- `tc.math.equal(a, b);` - Returns true if `a == b`.
- `tc.math.not_equal(a, b);` - Returns true if `a != b`.
- `tc.math.less_then(a, b);` - Returns true if `a < b`.
- `tc.math.greater_then(a, b);` - Returns true if `a > b`.
- `tc.math.greater_then_or_equal(a, b);` - Returns true if `a >= b`.
- `tc.math.less_then_or_equal(a, b);` - Returns true if `a <= b`.
- `tc.math.access(x);` - Accesses value from variable `x`.

### Variable Access
- `$var` - Explicit variable value access.
- `var` - Shorthand variable value access (when supported by context).

## Printing / Output
- `tc.printf(value);` - Prints value to console (CLUI).
- `tc.print(value);` - Alias of `tc.printf()`.

## Functions Runtime
- `tc.func.init();` - Initializes function runtime/context.
- `tc.ret.init();` - Prepares for return usage.

## Loops Runtime
- `tc.loops.init();` - Initializes loop control.

## Graphics / GUI
- `tc.graphics.init();` - Initializes the GUI subsystem.

## Code Injection (CI)
- `tc.lang.init();` - Initializes a code injection block.
- `tc.math.assign(tc.lang.CODE, 0b1);` - Activates code injection.
- `tc.math.assign(tc.lang.CODE, 0b0);` - Deactivates code injection.
- `tc.math.assign(tc.lang.STATUS, 0b1);` - Marks code ready to run.
- `tc.lang.run(tc.lang.STATUS);` - Runs injected code.

## Variables
- `tc.lang.CODE`
- `tc.lang.STATUS`
- `tc.graphics.UPDATE` - If 1 = the update function will run every tick and if 0 the update function is just a normal function. 

By: Nils Efverman, CEO FÈUE
