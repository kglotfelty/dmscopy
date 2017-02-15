
# dmscopy

This is a version of the [CIAO](http://cxc.cfa.harvard.edu/ciao)
[dmcopy](http://cxc.cfa.harvard.edu/ciao/ahelp/dmcopy.html) tool
that takes in a table along with a [stack](http://cxc.cfa.harvard.edu/ciao/ahelp/stack.html)
of [filters](http://cxc.cfa.harvard.edu/ciao/ahelp/dmfiltering.html) 
and a stack of output files.    Then in 1 pass through the input file
it applies each of the filters to create the outputs files all at once.


```bash
$ dmscopy acis_evt1.fits "[grade=0,2,3,4,6],[grade=5,7]" good_grades.fits,bad_grades.fits 
```

```bash
$ dmscopy acis_evt.fits "ccd_id=igrid(0:9:1)" "out_igrid(0:9:1).fits"
```

