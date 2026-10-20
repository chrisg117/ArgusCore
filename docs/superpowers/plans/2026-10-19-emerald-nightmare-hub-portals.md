# Emerald Nightmare hub portals Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:executing-plans (run natively; the user asked for the implementation directly).

**Goal:** Walk-in hub portals to the four wings, gated on Nythendra with a GM bypass, plus return triggers.

**Architecture:** One `AreaTriggerAI` (`at_en_hub_portal`) for the gated hub triggers; plain action-2 triggers for the returns; one SQL file.

**Tech Stack:** C++ (TrinityCore fork), MySQL update file.

**Spec:** `docs/superpowers/specs/2026-10-18-emerald-nightmare-hub-portals-design.md`

## Global Constraints

- Branch `dev`; commit only these files; no `--no-cache`; no restart with a client connected; ask the session holding `~/arguscore` before building.
- SQL needs an image rebuild (`~/emerald-nightmare/build.sh`, `boot_check.sh`; DBErrors baseline 13422).
- Ids: templates and create properties 152003-152010, areatrigger spawns 1520201-1520208, gameobject guids 1520201-1520204.

## Review Focus

- A non-player unit entering a trigger is ignored.
- A GM with Nythendra alive is allowed; a player with Nythendra alive is refused with a message.
- A return arrival must not land inside a hub trigger (portals sit 8 yd outside the ring spots).

### Task 1: Script

- Create `src/server/scripts/BrokenIsles/EmeraldNightmare/areatrigger_emerald_nightmare.cpp` (`at_en_hub_portal`, `AddSC_areatrigger_emerald_nightmare`).
- Modify `src/server/scripts/BrokenIsles/broken_isles_script_loader.cpp`: declare and call it in the Emerald Nightmare block.
- Verify: it builds with no warnings in the touched files.

### Task 2: SQL

- Create `sql/updates/world/master/2026_10_19_04_world.sql`.
- Verify: the world server boots, applies it, and `DBErrors.log` stays within the baseline.

### Task 3: In-game test

- Spec Testing steps 1-8, then correct positions and the wing mapping with `.gps`.
