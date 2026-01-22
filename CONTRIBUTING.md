# Contributing to OutOfThyme

Thank you for your interest in contributing to **OutOfThyme**! This project aims to create a faithful software emulation of the Bastl Thyme effects processor.

To ensure the emulation remains accurate to the hardware and the code remains stable, please review the following guidelines before submitting a Pull Request (PR) or Issue.

## 1. Project Philosophy

This is not just a delay plugin; it is a hardware emulation.

* **Accuracy First:** Logic changes should be justified by the Bastl Thyme manual or hardware behavior.
* **DSP Architecture:** We use a specific `TapeEngine` class that handles the separation of Read/Write heads. Please do not bypass this engine for standard processing.

## 2. How to Report Bugs

When filing an issue, please include:

1. **The Behavior:** What did you hear? (e.g., "The loop fades out when Freeze is on.")
2. **The Expectation:** What does the manual say should happen? (e.g., "Manual pg 14: Feedback sets to full automatically in Freeze.")
3. **Reproduction:** Steps to reproduce (e.g., "Set Mix to 100%, Enable Freeze").
4. **Environment:** OS, DAW, and Plugin Format (VST3/AU).

## 3. Branching & Workflow

* **Branch Naming:** Please use standard prefixes to categorize your work:
* `feature/` (New capabilities or DSP additions)
* `fix/` (Bug repairs)
* `docs/` (Documentation updates)
* `refactor/` (Code cleanup with no logic change)


* **Workflow:** Always work on a separate branch derived from `develop`.
* **Merging:** When submitting your PR, please ensure you **squash your commits** into a single clean commit and **delete the branch** after the merge is complete.

## 4. Pull Request Guidelines

* **One Feature per PR:** Do not mix GUI changes with DSP logic fixes in the same PR.
* **Code Style:** Follow the standard JUCE coding guidelines.
* Use `juce::dsp` classes where possible.
* Keep DSP logic inside `TapeEngine.cpp` and parameter handling in `PluginProcessor.cpp`.


* **Safety Checks:**
* If modifying `TapeEngine`, ensure you are not corrupting the filter state (we use separate instances for Output and Feedback paths).
* Ensure buffer allocations in `prepareToPlay` account for the maximum delay time (approx 2.7s + buffers).



## 5. Working on the GUI

* I am currently transitioning from Canva prototypes to JUCE `Component` implementation.
* If you are implementing a knob or button, please ensure it has a corresponding `Attachment` in the `PluginEditor`.

## 6. Resources

* [Bastl Thyme Manual](https://bastl-instruments.github.io/thyme/manual.html#/What_is_Thyme) - The source of truth.
* [JUCE Documentation](https://docs.juce.com/)

By contributing, you agree that your code will be licensed under the same license as the project.
