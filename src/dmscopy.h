/*                                                                
**  Copyright (C) 2016  Smithsonian Astrophysical Observatory 
*/                                                                

/*                                                                          */
/*  This program is free software; you can redistribute it and/or modify    */
/*  it under the terms of the GNU General Public License as published by    */
/*  the Free Software Foundation; either version 3 of the License, or       */
/*  (at your option) any later version.                                     */
/*                                                                          */
/*  This program is distributed in the hope that it will be useful,         */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*  GNU General Public License for more details.                            */
/*                                                                          */
/*  You should have received a copy of the GNU General Public License along */
/*  with this program; if not, write to the Free Software Foundation, Inc., */
/*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.             */
/*                                                                          */

#include "ascdm.h"
#include "dslib.h"
#include "stack.h"
#include "histlib.h"

#define I_AM "dmscopy"

typedef struct {
    char infile[DS_SZ_PATHNAME];
    char filters[DS_SZ_PATHNAME];
    char outfile[DS_SZ_PATHNAME];
    int clobber;
    int verbose;
    Stack filter_stack;
    Stack outfile_stack;    
    long num_filters;
} Parameters;


typedef struct {
    char *filter;
    char *outfile;    
    dmRowFilter *dm_filter;
    dmBlock *out_block;
} Filter;


/* ---  Prototypes  --------------------------------*/

int dmscopy(void);
Filter* setup_filters( dmBlock *inBlock, Parameters *pars);
Parameters* get_inputs(void);
Stack parse_stack( char *infile );
int process_infile( dmBlock *inBlock, Filter *filters, Parameters *pars );
int do_filters( dmBlock *inBlock, Filter *filters, Parameters *pars );
int do_outfiles( dmBlock *inBlock, Filter *filters, Parameters *pars );
int cleanup(Filter *filters, Parameters *pars );

