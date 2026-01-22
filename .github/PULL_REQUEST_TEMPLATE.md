## Description
## Related Issue
## Type of Change
- [ ] `fix/` (Bug fix)
- [ ] `feature/` (New feature or DSP addition)
- [ ] `docs/` (Documentation update)
- [ ] `refactor/` (Code cleanup, no logic change)

## Hardware Accuracy Check
* **Manual Reference:** * **Behavior:** ## Checklist
- [ ] My branch name follows the format `type/description` (e.g., `fix/tape-delay-time`).
- [ ] I have **squashed** my commits and am ready to delete this branch upon merge.
- [ ] I have not mixed GUI changes with DSP logic in this PR.
- [ ] **DSP Safety:** If I modified `TapeEngine`, I confirmed filter states are safe (separate instances for Output/Feedback).
- [ ] **DSP Safety:** Buffer allocations account for max delay time (approx 2.7s).