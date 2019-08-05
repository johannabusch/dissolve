# Develop Branch Worklist

## Prep
- Fix Analysis test case
- Spatial density function calculation
- 2D plotting
- 3D plotting of surfaces (marching cubes)

## Main
- Fix documentation (include path for mermaid)
- Convert argon example from quickstart guide.
- Add water example
- Add benzene example (inc. sdf/slicing comparison between equilibrated / refined simulations)
- Add silica example
- Convert quickstart guide to page in docs.

## Other
- [General] Make Module::frequency_ default to 1 in all cases?  Then suggests layers for shake / calc / refine, which is quite neat... Still allow adjusting.
- Control Layer: A layer that manages the execution of processing layers, enabling regeneration of configurations, minimisation, refinement, testing, fitting of parameters. With other proposed changes to allow procedure definitions for configuration creation, this could be the best route to allow searching through this type of parameter space. Can dump a restart file at each 'good' or 'best' point, rather than every 'n' iterations of the main loop. Control Layer could be present all the time, but the default option is just to continually iterate over the layers.
- [General] Harmonise command-line arguments between CLI and GUI versions (use gengetopt?)

## More
- Importing CIF structures as Species - break up into molecular units? Need to provide basic information on the contained units, probably as empirical formula (e.g. for CuBTC would be 'Cu' and 'C9O6H3'). This info can then be used to find, rotate, and re-map those molecules in a unit cell (folded molecularly). The Species then contains a list of copies of that particular molecular unit, at real coordinates.  These can then be pasted into a supercell, replicating the crystal. Need to have options in the AddSpecies node to utilise symmetry copies. A species may also contain multiple variants of coordinates in the form of conformers (potentially weighted by some factor, e.g. deltaE??). Store both these in a list of copies_, storing coordinates only (we will always use the basic Species information as a template for elements, bonds etc., as well as forcefield terms.
- [AS ISSUE] Control Layer: A layer that manages the execution of processing layers, enabling regeneration of configurations, minimisation, refinement, testing, fitting of parameters. With other proposed changes to allow procedure definitions for configuration creation, this could be the best route to allow searching through this type of parameter space. Can dump a restart file at each 'good' or 'best' point, rather than every 'n' iterations of the main loop. Control Layer could be present all the time, but the default option is just to continually iterate over the layers.
- Remove Bond::SoftHarmonic form.