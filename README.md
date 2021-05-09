# RfgTools++
Code for interacting the file formats of the PC versions of Red Faction Guerrilla. This is actively used by [Nanoforge](https://github.com/Moneyl/Nanoforge/). Special thanks to [Gibbed](https://github.com/gibbed/) who originally reverse engineered the packfile, texture, vint_doc, and asm_pc formats.

## Formats
The table below lists each format used by RFG and the degree that this library supports them. Look below the table for descriptions of each column.
| Format          | Extension(s)         | Read | Write |
|-----------------|----------------------|------|-------|
| Packfile        | vpp_pc, str2_pc      | ✔️    | ✔️     |
| Streaming index | asm_pc               | ✔️    | ✔️     |
| Texture         | *cpeg_pc, *cvbm_pc   | ✔️    | ✔️     |
| Static mesh     | *csmesh_pc           | ✔️    | ❌     |
| Character mesh  | *ccmesh_pc           | ✔️    | ❌     |
| Map zone        | rfgzone_pc, layer_pc | ✔️    | ❌     |

**Extension(s)** - The file extensions used for this format. Extensions prepended with an asterisk `*` are split into two files known as the cpu file and gpu file. For example, for static meshes `.csmesh_pc` is the cpu file extension and `.gsmesh_pc` is the gpu file extension. Typically the cpu file data resides only in cpu RAM and the gpu file data is loaded into VRAM for use by the gpu.

**Read** - If checked then data can be extracted from this format is a usable form. E.g. extracting vertices and indices from a mesh for use in Blender.

**Write** - If checked the format can resaved with modified data in a form usable by the game.
