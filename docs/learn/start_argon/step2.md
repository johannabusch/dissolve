---
title: Step 2 - Set up a Forcefield
grand_parent: Learn Dissolve
parent: Liquid Argon
nav_order: 2
---
# Liquid Argon

## 2. Set up a Forcefield

To simulate any system we need to supply suitable parameters to describe the interactions between atoms, both internally within the same species and externally between atoms of the same (and different) species. The _Forcefield_{: .text-green-100} tab is always available, and summarises the atom types used over all defined species and the resulting pair potentials, as well as master intramolecular definitions that we aren't using here. You can also set other important options, such as the range of the generated pair potentials and the truncation schemes.

We'll now describe the interactions for your argon species by taking the terms from one of Dissolve's [built-in forcefields](../../setup/forcefields.md). Return to the species tab for your argon atom, and do the following:

- Run the _Add Forcefield Terms_{: .text-green-100} wizard from the _Species_{: .text-green-100} menu (_Species→Add Forcefield Terms_{: .text-green-100}).
- From the available forcefields choose `OPLSAA2005/NobleGases` and click _Next_{: .text-green-100}.
- For more complicated molecular species you may want to change how (or which) _intra_molecular terms are generated. This can be done on the _Select Terms_{: .text-green-100} page but we have no need to do so for our argon atom, so just click _Next_{: .text-green-100}.
- Note that when assigning atom types to species, Dissolve will always check to see if the names of the new atom types conflict with any already defined in the _Forcefield_{: .text-green-100} tab. In the general case you have the option of renaming the new atom types to avoid such conflicts, or to simply overwrite the existing atom type parameters with the new values (_Overwrite parameters in existing AtomTypes_{: .text-green-100}). We have no such issues here, so click _Next_{: .text-green-100}.
- Similarly, if master intramolecular terms are to be created / imported they are also checked for naming conflicts. We have no intramolecular terms to worry about, so click _Finish_{: .text-green-100} to complete the wizard.

If you now return to the _Forcefield_{: .text-green-100} you will see in the _Atom Types_{: .text-green-100} section that we now have an atom type for our argon atom that contains the necessary interaction parameters. The short range type (_SR Form_{: .text-green-100} in the table) specifies the functional form of the van der Waals interaction parameters for each atom type, which in our case is `LJGeometric` indicating that the parameters reflect a standard Lennard-Jones potential utilising geometric combination rules. Note that all atom types are strictly associated with a chemical element, and can only be applied to that element within a species.

By default, Dissolve will generate all the necessary pair potentials for the current set of atom types automatically - these are listed in the _Pair Potentials_{: .text-green-100} section where you can select individual potentials and visualise them in the associated plot.

[Previous Step](step1.md){: .btn }   [Next Step](step3.md){: .btn }