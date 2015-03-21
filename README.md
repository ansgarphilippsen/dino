# dino
A Molecular Visualization Program, from the 90s, archived here.

I wrote this program as part of my PhD thesis at the University of Basel, Switzerland. It was
part of the second generation molecular visualization tools for proteins and other structural biology
data, with other promiment members being RasMol, VMD, Molekel, Chimera. These all appeared during the
initial 3D revolution, with SGI leading the field in semi-affordable 3D hardware (for an institution,
at least).

DINO still runs on Linux and OSX, but it has not been under active development for over a decade,
with very sporadic changes for a small group of dedicated users. It was last shown at the FEBS 2005
congress in Budapest.

DINO was used do produce countless figures for peer-reviewed publications, as - at the time - if offered
more than just a single protein representation; it supported surfaces made from patches, colored with
information from other protein patches; with iso-contours and grids, it had good scalar data
represenations; and with height maps it could visually combine surface scanning microscpy data (as from
atomic force microscopy) with proteins. It was among the first packages to embrace hybrid concepts in
structural biology, years before the term became official.

DINO's command line syntax is tcl and that turned out to be perhaps an unfortunate choice; once pymol
and other python-based tools started appearing in the structural biology community, and it became clear 
that this was a good trend, a conscious decision was made to leave the program, with its rich feature set,
as is. Also, I, the author, had moved on to other projects and responsibilities.

At the time the legal status of the source-code, having been developed while I was employed as a
PhD student, was unclear, and legal services from the university were unable to give thumbs up for
any sort of release other than binary.

It is time to move the source code outside my private SVN repository and into the open, mostly
to archive this endavour of mine at the beginning of my career. I cloned the whole revision history 
as much as I still have it, mostly to be able to smile at some of the messages.

