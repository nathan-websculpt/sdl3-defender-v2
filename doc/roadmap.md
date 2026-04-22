## TODO

- validate linux runtime path
- continue windows release-gate
- centralize gameplay constants
- remove unused
- typo / cleanup spots
- finally decide on formatting
- add flag to ./analyze.ps1
    - -VcpkgRoot "C:\vcpkg\dir"
- swapped from all-caps const to k-prefix somewhere along the way: normalize
- tighten render immutability boundary  
    - rendering currently treats gameplay state as read-only via `const GameStateData&`, but this is not enforced due to pointer-owned entities. Improve const-correctness to make the boundary type-enforced rather than convention-based