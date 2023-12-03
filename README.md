## Introduction

The core functions are available.

It has many problems now, so it's not recommended to use it in the production environment.

**Maybe it will be refactored in the future.**

## Road

1. convert on read ?
  when read the value, convert it just on time.

2. string holder?
  use a holder to hold the maintaining string, and use string_view in the json class.

## Todo

- [ ] boost perf
- [ ] more tests to cover more cases
- [ ] use string_view instead of string (change copy mode to span mode)

## Update

### a simple benchmark for Modern JSON

About Modern JSON library:
 - [Home Page](https://json.nlohmann.me/)
 - [github](https://github.com/nlohmann/json)

>note:
> My lib is a incomplete support for json operators, there are many bugs.
> And Modern JSON support much more methods(actually humanize and useable).
> And I use some opportunistic mode to parse json(made a mixture of lexer and parser, it made it less readable but enhanced performance).

```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
Modern        1390987 ns      1385872 ns          479
My             718980 ns       716968 ns          889
```

### benchmark for 0.1 to 0.2

```shell
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
Old           4616645 ns      4578780 ns          159
New            977329 ns       973994 ns          667
```

## Usage

It's recommended to have a look at the `tests/json.cpp` file, all the simple example are over there.

## Docs

The apis iterate rapidly, so it is just blank now.

## License

MIT