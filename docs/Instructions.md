
Instruction have always the same format:

| byte-offset | Value/Meaning                                        |
|-------------|------------------------------------------------------|
| 0           | Instruction ID: What to generate (see table)         |
| 1-2         | Length: How many samples should be generated?        |
| 3- 5n       | parameter0, parameter1, ... (up to four parameter) |

The first bit of the Instruction specifies the channel (0 = X, 1 = Y), the instruction is applied to.

It's expected that every channels is completely filled (nor over oder under flowed)

All values (except the Instruction ID, this is 8 bit value) are 16 bit values, encode as big endian like this:
```c
buffer[offset + 0] = (value & 0xff00) >> 8;
buffer[offset + 1] = (value & 0x00ff) >> 8;
```

### Instruction Types
| ID     | Type      | Parameters                               |
|--------|-----------|------------------------------------------|
| `0x00` | NONE      | -                                        |
| `0x01` | CONST     | level                                    |
| `0x02` | LINE      | fromValue, toValue                       |
| `0x03` | QUADRATIC | fromValue, toValue, CTRL                 |
| `0x04` | CUBIC     | fromValue, toValue, CTRL0, CTRL1         |
| `0x05` | SINE      | lowValue, highValue, period, phase       |

(CUBIC and QUADRATIC refer to [Bézier Curves](https://en.wikipedia.org/wiki/B%C3%A9zier_curve), see [desmos](https://www.desmos.com/calculator/q5xuopyfo6))