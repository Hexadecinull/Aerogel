# Contributing

## Code Style

- C: K&R brace style, 4-space indent, `snake_case`
- C++: same as C plus `PascalCase` for types
- Assembly: 4-space indent, uppercase mnemonics
- Rust: `rustfmt` defaults

## Comments

Only comment non-obvious intent. Do not narrate code that reads clearly on its own.

## Commit Messages

```
component: short imperative description

Optional body explaining why, not what.
```

Examples:
```
arch/x86/boot: enable A20 via fast method with keyboard fallback
kernel/pmm: fix off-by-one in bitmap boundary check
drivers/ps2: handle translation mode for scancode set 1
```

## Branches

- `main` — stable, CI must pass
- `dev`  — integration branch
- Feature branches: `feature/description`
- Fix branches: `fix/description`

## Pull Requests

- Target `dev`, not `main`
- Must pass both x86 and x86_64 CI builds
- One logical change per PR
