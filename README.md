# AI2 Assignment 1 — D4-V2 Search and Rescue with PDDL/PDDL+

<p align="center">
  <b>PDDL · PDDL+ · Search and Rescue · Unknown Victim Location · Health Degradation · ENHSP</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Course-AI2-blue" />
  <img src="https://img.shields.io/badge/Language-PDDL-orange" />
  <img src="https://img.shields.io/badge/Extension-PDDL%2B-green" />
  <img src="https://img.shields.io/badge/Planner-ENHSP-lightgrey" />
  <img src="https://img.shields.io/badge/Theme-D4--V2-red" />
</p>

---

This repository contains the final submission for **AI2 Assignment 1 — D4-V2: Search and Rescue – Unknown Victim Location**.

The project models a single rescue robot operating in a damaged building. The topology is known, but the victim location is not directly available to the robot. The robot must explore, inspect rooms, detect the victim, and complete rescue operations while time affects the victim's health.

The repository contains **three related versions** of the same scenario. This is intentional. The assignment uses the word **rescue**, but the word can reasonably be interpreted in more than one way:

```text
rescue = treat the victim on site?
rescue = evacuate the victim to the base?
rescue = decide between transport and stabilization depending on health?
```

The final repository therefore keeps the required core model and two additional extensions that document how the modelling decision changes the planning task.

| Version | Folder | Role |
|---|---|---|
| Core assignment | `codes/D4-V2_search_and_rescue/` | Required Q1/Q2 model. Rescue is treated as an on-site rescue action after detection. |
| Transport variant | `codes/D4-V2_search_and_rescue_variant/` | Optional interpretation. Rescue means loading the victim, transporting the patient to base, and unloading there. |
| Definitive model | `codes/D4-V2_search_and_rescue_definitive/` | Final extended version. The robot assesses health and autonomously chooses direct transport, stabilization, or failure. |

The version to check first is:

```text
codes/D4-V2_search_and_rescue_definitive/
```

It is the most complete model because it combines exploration, detection, health degradation, patient assessment, transport, stabilization, mission deadline logic, and a too-late unsolvable case.

---

## Table of Contents

| Section | Description |
|---|---|
| [1. Quick Start](#1-quick-start) | How to inspect the repository and identify the final model |
| [2. Repository Structure](#2-repository-structure) | Folder-level organization |
| [3. Assignment Scenario](#3-assignment-scenario) | Natural-language problem definition |
| [4. Why There Are Three Versions](#4-why-there-are-three-versions) | The modelling ambiguity around the word `rescue` |
| [5. Core Assignment Model](#5-core-assignment-model) | Required Q1/Q2 deliverable |
| [6. Transport-to-Base Variant](#6-transport-to-base-variant) | Optional evacuation interpretation |
| [7. Definitive PDDL+ Model](#7-definitive-pddl-model) | Final extended model and autonomous branch selection |
| [8. Notes and Discussion Files](#8-notes-and-discussion-files) | Where the modelling rationale is documented |
| [9. Planner and Validation Notes](#9-planner-and-validation-notes) | Planner behaviour and expected outcomes |
| [10. What Is Extra Compared to the Assignment](#10-what-is-extra-compared-to-the-assignment) | Extensions beyond the minimum requirements |
| [11. Final Reading Order](#11-final-reading-order) | Recommended review sequence |

---

# 1. Quick Start

From the repository root:

```bash
# Move to the repository root used for the final AI2 submission.
cd /home/ruggio/Documents/UniGe/AI2/AI2_Assignment1

# Inspect the top-level structure before checking the individual models.
tree -L 3
```

The main folder to review first is:

```bash
# Open the final extended model in VS Code.
code codes/D4-V2_search_and_rescue_definitive
```

The core assignment folder is still included and should be considered the minimum required deliverable:

```bash
# Open the official Q1/Q2 assignment solution.
code codes/D4-V2_search_and_rescue
```

---

# 2. Repository Structure

```text
AI2_Assignment1/
├── codes/
│   ├── D4-V2_search_and_rescue/
│   │   ├── Q1_basic_pddl/
│   │   ├── Q2_pddl_plus/
│   │   ├── notes/
│   │   └── README.md
│   ├── D4-V2_search_and_rescue_variant/
│   │   ├── domain_variant_plus.pddl
│   │   ├── problem_variant_fast.pddl
│   │   ├── problem_variant_delayed.pddl
│   │   ├── plan_variant_fast.txt
│   │   ├── plan_variant_delayed.txt
│   │   ├── notes/
│   │   └── README.md
│   └── D4-V2_search_and_rescue_definitive/
│       ├── domain_definitive_plus.pddl
│       ├── problem_definitive_direct_transport.pddl
│       ├── problem_definitive_stabilize_then_transport.pddl
│       ├── problem_definitive_too_late.pddl
│       ├── plan_definitive_direct_transport.txt
│       ├── plan_definitive_stabilize_then_transport.txt
│       ├── plan_definitive_too_late.txt
│       ├── notes/
│       └── README.md
├── Report/
├── slide/
└── README.md
```

| Area | Meaning |
|---|---|
| `codes/` | All executable PDDL/PDDL+ models and saved planner outputs |
| `Report/` | Final written report material |
| `slide/` | Presentation material |
| root `README.md` | High-level explanation of the repository and modelling choices |

---

# 3. Assignment Scenario

The assigned scenario is **D4-V2: Search and Rescue – Unknown Victim Location**.

A single rescue robot operates in a damaged building. The robot knows the topology of the building, but the victim location is initially unknown. Therefore, the robot cannot simply move to the victim and rescue them. It must first acquire information by exploring and inspecting rooms.

| Scenario element | Modelled as |
|---|---|
| Robot | One mobile rescue robot, named `rescuebot` in the models |
| Environment | Known graph of rooms and connections |
| Unknown victim location | Approximated through explicit inspection and detection predicates |
| Exploration | Movement between connected rooms + room inspection |
| Detection | Symbolic fact produced after inspecting the correct room |
| Rescue | Enabled only after detection |
| Time pressure | Victim health decreases in PDDL+ models |

The intended causal chain is:

```text
move → inspect → detect victim → rescue
```

In the extended models, the chain becomes:

```text
move → inspect → detect victim → assess health → choose rescue strategy → complete mission
```

---

# 4. Why There Are Three Versions

The main modelling issue was the meaning of the word **rescue**.

The assignment requires rescue operations after the victim has been found, but it does not strictly define whether rescue ends when the robot reaches the victim or whether the robot must physically evacuate the victim to a safe location.

This ambiguity produced three versions.

| Interpretation of `rescue` | Consequence in PDDL/PDDL+ | Implemented in |
|---|---|---|
| Rescue means reaching and helping the victim on site | The final action can be `rescue` after `victim-detected` | `D4-V2_search_and_rescue` |
| Rescue means evacuation to the base | The robot must load the patient, move while carrying them, and unload at base | `D4-V2_search_and_rescue_variant` |
| Rescue strategy depends on the patient's condition | The robot must assess health and choose direct transport or stabilization before transport | `D4-V2_search_and_rescue_definitive` |

## 4.1 Why the core version is kept

The core version is the closest match to the official assignment requirements.

It contains:

| Requirement | Implemented in core model |
|---|---|
| Q1 Basic PDDL | `Q1_basic_pddl/` |
| Known-location instance | `problem_known_location.pddl` |
| Exploration instance | `problem_exploration.pddl` |
| Explicit inspection | `inspect-empty-room`, `inspect-victim-room` |
| Detection before rescue | `victim-detected` enables `rescue` |
| Q2 PDDL+ health degradation | `Q2_pddl_plus/domain_plus.pddl` |
| Fast/delayed comparison | `problem_fast_rescue.pddl`, `problem_delayed_rescue.pddl` |

## 4.2 Why the variant is kept

The transport-to-base variant is kept because it explores a more realistic interpretation of rescue.

In this version, it is not enough to detect the victim. The robot must physically evacuate the patient.

```text
detect victim → load patient → move with patient → unload patient at base
```

This adds modelling complexity because movement changes depending on whether the robot is carrying the patient.

## 4.3 Why the definitive model is kept

The definitive model is kept as the final extended version because it resolves the ambiguity by making the rescue strategy depend on the patient's condition.

The robot does not follow a fixed script. It assesses the patient and then the planner selects the valid branch according to numeric health thresholds and mission timing.

| Health/time condition | Expected branch |
|---|---|
| Health is high enough | Direct transport to base |
| Health is low but still recoverable | Stabilize patient, then transport to base |
| Mission starts too late or deadline is exceeded | No valid rescue plan |

This is the most complete version, but it is also explicitly an extension beyond the minimum assignment.

---

# 5. Core Assignment Model

Folder:

```text
codes/D4-V2_search_and_rescue/
```

This folder contains the required Q1 and Q2 deliverable.

## 5.1 Q1 — Basic PDDL

Folder:

```text
codes/D4-V2_search_and_rescue/Q1_basic_pddl/
```

Files:

```text
domain.pddl
problem_known_location.pddl
problem_exploration.pddl
plan_known_location.txt
plan_exploration.txt
```

Q1 approximates the unknown victim location problem using classical PDDL.

| Modelling choice | Purpose |
|---|---|
| Rooms as graph nodes | Makes navigation a symbolic graph-search problem |
| `connected` predicate | Defines valid movements between rooms |
| `move` action | Changes the robot's current room |
| `inspect-empty-room` | Marks a non-victim room as inspected |
| `inspect-victim-room` | Produces the detection fact when the victim room is inspected |
| `victim-detected` | Represents information acquired by the robot |
| `rescue` | Becomes applicable only after detection |

The important distinction is:

```text
victim-at       = where the victim actually is in the encoded world
victim-detected = what the robot has discovered through inspection
```

This separation is necessary because classical PDDL does not implement true partial observability. The model still contains a complete symbolic initial state, but the rescue action is not allowed to use the hidden victim location directly.

## 5.2 Q2 — PDDL+

Folder:

```text
codes/D4-V2_search_and_rescue/Q2_pddl_plus/
```

Files:

```text
domain_plus.pddl
problem_fast_rescue.pddl
problem_delayed_rescue.pddl
plan_fast_rescue.txt
plan_delayed_rescue.txt
```

Q2 extends the model with time-dependent health degradation.

| PDDL+ element | Meaning in the rescue scenario |
|---|---|
| `victim-health` | Numeric fluent representing the victim's remaining health |
| `health-degradation` | Process that decreases health over time |
| `victim-dies` | Event triggered automatically when health reaches zero |
| Fast rescue problem | Rescue should succeed before health reaches zero |
| Delayed rescue problem | Delay should make rescue fail or become semantically invalid |

The key point is that in PDDL+, doing nothing is no longer neutral. Time passes, processes remain active, and the victim's health decreases while the robot moves, inspects, waits, or delays.

---

# 6. Transport-to-Base Variant

Folder:

```text
codes/D4-V2_search_and_rescue_variant/
```

This folder contains the optional interpretation where rescue means **evacuation**.

The variant adds transport-specific actions:

| Action | Purpose |
|---|---|
| `start-load-patient` | The robot loads the detected patient |
| `start-move-with-patient` | The robot moves while carrying the patient |
| `start-unload-patient-at-base` | The robot unloads the patient at the safe base |

The final goal requires both rescue completion and the patient being at the base.

```text
victim detected → patient loaded → robot moves with patient → patient at base → rescued
```

This version is **not a replacement** for the core model. It documents a richer interpretation of the same scenario.

---

# 7. Definitive PDDL+ Model

Folder:

```text
codes/D4-V2_search_and_rescue_definitive/
```

This is the final extended version to check first.

Files:

```text
domain_definitive_plus.pddl
problem_definitive_direct_transport.pddl
problem_definitive_stabilize_then_transport.pddl
problem_definitive_too_late.pddl
plan_definitive_direct_transport.txt
plan_definitive_stabilize_then_transport.txt
plan_definitive_too_late.txt
notes/modelling_choices.md
notes/discussion_sensing_time.md
notes/discussion_partial_observability.md
```

## 7.1 Main idea

The definitive model follows this pipeline:

```text
search → inspect → detect victim → assess patient → choose branch → transport to base
```

The branch is selected through numeric conditions on `victim-health` and mission timing.

| Case | Problem file | Expected result |
|---|---|---|
| Direct transport | `problem_definitive_direct_transport.pddl` | Plan exists; no stabilization action appears |
| Stabilize then transport | `problem_definitive_stabilize_then_transport.pddl` | Plan exists; stabilization appears before transport |
| Too late | `problem_definitive_too_late.pddl` | No valid rescue plan |

## 7.2 Why stabilization exists

Stabilization was added to model the idea that a patient with low health may not be safe to transport immediately.

This creates a planning choice:

```text
high health → transport directly
low health  → stabilize first, then transport
zero health → failure
```

## 7.3 Why mission deadline exists

During testing, the planner could wait artificially before acting. Since `victim-health` decreases over time, arbitrary waiting could push the model into the stabilization branch even when direct transport should have been selected.

The definitive model therefore includes mission clock/deadline logic. This prevents unrealistic waiting and keeps the branch selection meaningful.

---

# 8. Notes and Discussion Files

The notes are part of the explanation of the project. They are not secondary files; they document why the model is written in this way.

| File | What it explains |
|---|---|
| `codes/D4-V2_search_and_rescue/notes/modelling_choices.md` | Overall scenario, classical approximation, inspection, detection, rescue, health degradation |
| `codes/D4-V2_search_and_rescue/notes/discussion_partial_observability.md` | Why Q1 is only an approximation of unknown victim location |
| `codes/D4-V2_search_and_rescue/notes/discussion_sensing_time.md` | How inspection and time interact in Q2 |
| `codes/D4-V2_search_and_rescue_variant/notes/variant_transport_to_base_notes.md` | Why the transport-to-base interpretation was added |
| `codes/D4-V2_search_and_rescue_definitive/notes/modelling_choices.md` | Why the final model includes assessment, branch selection, stabilization, transport, and deadline logic |

The most important modelling limitation is that classical PDDL cannot represent true partial observability. The project therefore separates the real world fact `victim-at` from the robot's acquired information `victim-detected`.

---

# 9. Planner and Validation Notes

The validated setup used during the project was **ENHSP local Docker** for PDDL+ models.

| Model | Planner status |
|---|---|
| Q1 Basic PDDL | Valid plans saved for known-location and exploration instances |
| Q2 PDDL+ fast rescue | Valid successful output documented |
| Q2 PDDL+ delayed rescue | Discussed as a model-level failure when delay exceeds available health |
| Variant fast case | Documented as successful |
| Variant delayed case | Kept as documented failure/unsupported-output case |
| Definitive direct transport | Valid plan; direct transport selected |
| Definitive stabilize then transport | Valid plan; stabilization selected |
| Definitive too late | Correctly unsolvable |

## 9.1 Planner-output caveat

Some PDDL+ planner outputs can be syntactically returned while being semantically suspicious, especially when an empty plan is returned for a non-empty goal. Those cases are documented rather than hidden.

The relevant interpretation is the model-level one:

```text
if the goal contains predicates that are false in the initial state,
then an empty plan cannot be considered a valid solution.
```

---

# 10. What Is Extra Compared to the Assignment

The minimum assignment asks for Q1 Basic PDDL, Q2 PDDL+ health degradation, valid plans, and a discussion of partial observability and sensing/time.

This repository includes additional material.

| Extra element | Why it was added |
|---|---|
| Transport-to-base variant | To address the ambiguity of the word `rescue` |
| Patient loading/unloading | To model evacuation instead of only on-site rescue |
| Movement while carrying patient | To make transport causally different from normal movement |
| Patient assessment | To make the robot choose a strategy based on health |
| Stabilization branch | To represent low-health rescue before transport |
| Mission clock/deadline | To prevent artificial planner waiting from changing the intended branch |
| Too-late unsolvable case | To validate that failure is possible when rescue starts too late |
| Dedicated notes | To make modelling choices and limitations explicit |
| Multiple READMEs | To make each version understandable independently |

These elements are intentionally kept in the repository because they show the modelling evolution from the official assignment to a richer PDDL+ rescue domain.

---

# 11. Final Reading Order

Recommended review order:

| Step | Read/check | Reason |
|---|---|---|
| 1 | This root `README.md` | Understand the repository and the three-version structure |
| 2 | `codes/D4-V2_search_and_rescue/` | Check the required Q1/Q2 assignment deliverable |
| 3 | `codes/D4-V2_search_and_rescue/notes/` | Check the official modelling discussion |
| 4 | `codes/D4-V2_search_and_rescue_variant/` | Check the optional transport interpretation |
| 5 | `codes/D4-V2_search_and_rescue_definitive/` | Check the final extended model |
| 6 | `Report/` | Read the final written explanation |
| 7 | `slide/` | Review the presentation material |

The final model to inspect first remains:

```text
codes/D4-V2_search_and_rescue_definitive/
```

The core assignment deliverable remains:

```text
codes/D4-V2_search_and_rescue/
```
