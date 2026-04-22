## notes

- window events are part of simulation input
- worried that sin / cos / interpolation is not guaranteed bitwise identical
    - due to compiler / cpu differences
- any code with a non-const Game& can obtain a mutable reference to the entire gameplay state
    - mutation boundary is not strongly enforced by the type / interface boundary
        - seems fine for a small game; should fix it though