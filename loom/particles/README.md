# Loom Blender Authoring Tool

A lightweight HTML/JS composer for wiring Weave Blender graphs (particles plus general-purpose math/utility nodes). Build graphs visually, then copy the generated C++ snippet directly into your `ParticleMachine` or plain `Blender` setup.

## Getting Started

1. From the repo root run a local static server (or just open the file directly):
   ```bash
   python -m http.server 9000
   # then visit http://localhost:9000/loom/particles/
   ```
   Opening `loom/particles/index.html` in a browser also works for a quick preview.
2. Use the **Nodes** palette to add particle or general nodes onto the canvas.
3. Drag nodes to arrange them. Select a node to edit its defaults, rename it, or toggle whether it acts as a root flow source.
4. Use the connection form (or the inspector’s outgoing list) to connect nodes, mirroring the flow ordering you want in the Blender graph.
5. Click **Generate C++** to produce setup code. The tool lists required includes, node creation, default value assignments, root flow links, and flow connections.
6. Copy the code block into your engine code (for example, inside a demo block similar to `WeaveTests.cpp`).

## Features

- Preconfigured support for particle nodes (Emitter, Sphere Init, Velocity Init, Physics Init, Transform Init, Commit, Aging Sim, Physics Sim) plus general categories such as math, conditional, conversion, easing, RNG, and animation samplers.
- Editable default values for every node input plus optional node-specific toggles (e.g., sphere hemisphere constraints).
- Visual flow wiring with curved connection lines and root badges.
- Deterministic variable naming with automatic collision avoidance.
- Generated code includes the `wb` alias, node creation, input defaults, and flow hookups so you can paste it without manual editing (with `wp` included automatically when particle nodes are present).

## Extending

To expose additional nodes, edit `loom/particles/app.js`, append to the `NODE_LIBRARY` array, and refresh the page. Each entry is data-driven, so you can map new inputs, include paths, and custom options without touching the UI code.
