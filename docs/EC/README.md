# Error Code

Error code is a 64 bit code that is used to indicate an error in the system. It is used to indicate the type of error, the severity of the error, and the subsystem that caused the error. The error code is used to help developers and users understand what went wrong and how to fix it. This is working in progress and may change in the future. This is also not a final design and may change in the future. This is also not a final implementation and may change in the future. Use with caution. In the () is the letter/number in the error code at that possistion. When x'es shows up it just can be ANYTHING 1-9 or a-f (When it isen't in the 0x or 1x). It is just internal debug data that can be used to help developers understand what went wrong. 

## Positive (0)

* [Positive Error Codes](Positive/README.md)

## Negative (1)

* [Negative Error Codes](Negative/README.md)

## Dev

* [Dev](dev/README.md)

## Error codes Layout

The error codes are 64/256 bit.

Base for all error codes:

\[Negative; 1 bit = 1 negative, 0 positive]
\[DefaultHex; 1 bit = x hex / b binary / t base ten]
\[Severity; 2 bits]
\[SubseverityInfo; 2 bits]
\[Subsystem; 10 bits]
\[Subsubsystem; 6 bits]
\[Mode; 4 bits]
\[Args; 8 bits]
\[ErrorCode; 28 bits]
\[OS\_Other; 2 bits]
