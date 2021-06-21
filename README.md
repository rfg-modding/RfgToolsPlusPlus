# RfgTools++
Code for interacting the file formats of the PC versions of Red Faction Guerrilla. This is actively used by [Nanoforge](https://github.com/Moneyl/Nanoforge/). Special thanks to [Gibbed](https://github.com/gibbed/) who originally reverse engineered the packfile, texture, vint_doc, and asm_pc formats.

## Formats
The table below lists each format used by RFG and how much this library supports them. Extensions prepended with an asterisk `*` are split into two files known as the cpu file and gpu file. For example, for static meshes `.csmesh_pc` is the cpu file extension and `.gsmesh_pc` is the gpu file extension.

| Format                     | Extension(s)         | Read | Write |
|----------------------------|----------------------|------|-------|
| Packfile                   | vpp_pc, str2_pc      | ✔️    | ✔️     |
| Streaming index            | asm_pc               | ✔️    | ✔️     |
| Texture                    | *cpeg_pc, *cvbm_pc   | ✔️    | ✔️     |
| Static mesh                | *csmesh_pc           | ✔️    | ❌     |
| Character mesh             | *ccmesh_pc           | ✔️    | ❌     |
| Map zone                   | rfgzone_pc, layer_pc | ✔️    | ❌     |
| Vehicle mesh               | *ccar_pc             | ❌    | ❌     |
| Animation                  | anim_pc              | ❌    | ❌     |
| Rig                        | rig_pc               | ❌    | ❌     |
| Destructible object        | *cchk_pc             | ❌    | ❌     |
| Visual effect              | *cefct_pc            | ❌    | ❌     |
| Foliage mesh               | cfmesh_pc            | ❌    | ❌     |
| Terrain clutter mesh       | *cstch               | ❌    | ❌     |
| Terrain zone               | *cterrain_pc         | ❌    | ❌     |
| Terrain subzone            | *ctmesh_pc           | ❌    | ❌     |
| Fullscreen map data        | fsmib                | ❌    | ❌     |
| Shader                     | fxo_kg               | ❌    | ❌     |
| Steam localization strings | le_strings           | ❌    | ❌     |
| Localization strings       | rfglocatext          | ✔️     | ❌     |
| Render material            | mat_pc               | ❌    | ❌     |
| Animation Morph            | morph_pc             | ❌    | ❌     |
| UI                         | vint_doc             | ❌    | ❌     |
| Cloth sim                  | sim_pc               | ❌    | ❌     |
| Sound config               | xgs_pc               | ❌    | ❌     |
| Soundbank                  | xsb_pc               | ❌    | ❌     |
| Wavebank                   | xwb_pc               | ❌    | ❌     |
| Audio categories           | aud_pc               | ❌    | ❌     |
| Font                       | vf3_pc               | ❌    | ❌     |
| ?                          | vfdvp_pc             | ❌    | ❌     |
| ?                          | rfgvp_pc             | ❌    | ❌     |


