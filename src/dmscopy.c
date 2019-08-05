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


#include <stdlib.h>
#include <stdio.h>
#include "dmscopy.h"


/* Process the input stacks.  Check that stk build works, and that stack
 * is not empty
 */
Stack parse_stack( char *infile )
{
    Stack out_stack;
    
    out_stack = stk_build( infile);
    if ( NULL == out_stack ) {
        err_msg("ERROR: Unable to parse stack '%s'",infile);
        return(NULL);
    }
    if ( (0 == strlen( infile)  ) ||
         (0 == stk_count( out_stack )) ||
         ((1 == stk_count(out_stack) && ( 0 == strlen(stk_read_num(out_stack,1))) ))) {
             err_msg("ERROR: stack '%s' cannot be empty", infile);
             return(NULL);
    }
    stk_rewind(out_stack);
    
    return(out_stack);
}


/* Get input parameters from .par file.  Check that the 
 * filter stack and the outfile stack have the same size
 */
Parameters* get_inputs(void)
{
    Parameters *pars = (Parameters*)calloc(1,sizeof(Parameters));

    clgetstr("infile", pars->infile, DS_SZ_PATHNAME );
    clgetstr("filters", pars->filters, DS_SZ_PATHNAME );
    clgetstr("outfile", pars->outfile, DS_SZ_PATHNAME);
    pars->clobber = clgeti("clobber");
    pars->verbose = clgeti("verbose");
    
    /* Expand filter and outfile stacks */
    if ( NULL == (pars->filter_stack = parse_stack( pars->filters))) {
        free(pars);
        return(NULL);
    }

    if ( NULL == (pars->outfile_stack = parse_stack( pars->outfile))) {
        free(pars);
        return(NULL);
    }
              
    if ( stk_count(pars->outfile_stack) != stk_count(pars->filter_stack)) {
        err_msg("ERROR: Please specify the same number of outfiles (%d) as filters (%d)",
          stk_count(pars->outfile_stack), stk_count(pars->filter_stack));
        free(pars);
        return(NULL);
    }

    pars->num_filters = stk_count( pars->filter_stack );
    stk_rewind( pars->filter_stack);
    stk_rewind( pars->outfile_stack);    
    return(pars);
}


/*
 * Parse the dm filters. 
 * 
 * The filter string must be enclosed in []'s so we add them if they are
 * not there.
 * 
 * Also note that the filter alloc doesn't seem too concerned about the
 * filter actually being valid :(  Not sure how to catch syntax errors
 * here.
 * 
 * 
 */ 
int do_filters( dmBlock *inBlock, Filter *dm_filters, Parameters *pars )
{
    long ii;
    for (ii=0;ii<pars->num_filters;ii++) {    
        char *ff = stk_read_num(pars->filter_stack, ii+1);
        
        char *filter = (char*)calloc(strlen(ff)+3, sizeof(char));

        // Make sure filter is wrapped in []'s 
        strcpy(filter,ff);
        if (filter[0] != '[') {
            char *buff = (char*)calloc( strlen(ff)+3,sizeof(char));
            strcpy( buff, "[");
            strcat(buff, filter );
            strcpy(filter, buff );
            free(buff);
        }
        if (filter[strlen(filter)-1] != ']') {
            strcat( filter, "]");
        }
        stk_read_free(ff);

        // Parse filter string
        if ( NULL == (dm_filters[ii].dm_filter = dmTableAllocRowFilter( inBlock, filter))) {
            // TODO : Get DM specific error message here 
            // TODO: This doesn't seem to care about syntax errors in filter?
            err_msg("ERROR: Unable to parse filter '%s'", filter);                
            return(1);
        } 
        dm_filters[ii].filter = filter;
    } // end for ii
    
    return(0);
}


/* Setup the output DM blocks.
 * 
 * Go ahead and write all the header stuff before the data are written.
 * (good FITS practice due to FITS buffering)
 */
int do_outfiles( dmBlock *inBlock, Filter *dm_filters, Parameters *pars )
{
    // Create / setup output files
    Header_Type *hdr = getHdr( inBlock, hdrDM_FILE);

    long ii;
    for (ii=0;ii<pars->num_filters;ii++) {
        char *outfile = stk_read_num( pars->outfile_stack, ii+1);
        if (ds_clobber(outfile, pars->clobber, NULL) != 0 ) {
            return(1);
        }        
        dmDataset *ds = dmDatasetCreate( outfile );
        if ( NULL == ds ) {
            err_msg("ERROR: Problem creating outfile '%s'", outfile );
            return(1);
        }

        dmBool copy_data = FALSE;
        if ( NULL == ( dm_filters[ii].out_block = dmBlockCreateCopy( ds, NULL,  inBlock, copy_data ))) {
            err_msg("ERROR: Problem creating outfile '%s'", outfile );
            return(1);
        }        
        dm_filters[ii].outfile = outfile;

        // Take care of header stuff now before data are written
        put_param_hist_info( dm_filters[ii].out_block, I_AM, NULL, pars->verbose);

        // TODO:  Need to add the filter into the outfiles subspace.  How???


        /* If a FITS file, add header to null primary */
        if ( 1 != dmDatasetGetCurrentBlockNo( ds ) ) {
            dmBlock *pblock = dmDatasetMoveToBlock( ds, 1 ); // Null Primary 
            putHdr( pblock, hdrDM_FILE, hdr, PRIMARY_STS, I_AM);
            dmBlockClose(pblock);
        }
    } // end for ii

    freeHdr( hdr );

    return(0);
}



/* Setup the array of Filter structs that contian the DM filter and the
 * output blocks
 */
Filter* setup_filters( dmBlock *inBlock, Parameters *pars )
{    
    Filter *dm_filters = (Filter*)calloc( pars->num_filters, sizeof(Filter));    
    if ( NULL == dm_filters) {
        err_msg("ERROR: Allocing memory");
        return(NULL);
    }

    if ( 0 != do_filters( inBlock, dm_filters, pars ) ) {
        free(dm_filters);
        return(NULL);
    }

    if ( 0 != do_outfiles( inBlock, dm_filters, pars )) {
        free(dm_filters);
        return(NULL);        
    }

    return(dm_filters);    
}


/*
 * Loop over rows in the input file and redirect outputs.
 * 
 * Note: same row may end up in multiple output files.  That might be
 * what the user wants.  Could add a warning/check if we wanted too.  
 * (Is not an error condition.)
 * 
 *  
 */
int process_infile( dmBlock *inBlock, Filter *filters, Parameters *pars )
{
    void *row = dmTableAllocRow( inBlock );    
    long num_rows = dmTableGetNoRows( inBlock );

    long ii;
    for (ii=0;ii<num_rows;ii++) {
        dmTableGetRow( inBlock, row ); // This advances row counter        
        long jj;
        for (jj=0;jj<pars->num_filters;jj++) {            
            if ( dmTableFilterRow( inBlock, filters[jj].dm_filter, row ) != 0 ) {
                dmTablePutRow( filters[jj].out_block, row ); // Also advances row counter
            }
        } // end for jj (loop over filters )                
    } // end for ii (loop over rows )
    dmFree(row);


    return(0);
}

/*
 * Cleanup inputs/memory
 */
int cleanup(Filter *filters, Parameters *pars )
{
    /* Close output files -- use TableClose() to close block and dataset */
    long jj;
    for (jj=0;jj<pars->num_filters;jj++) {            
        dmRowFilterFree( filters[jj].dm_filter );
        dmTableClose( filters[jj].out_block );
        stk_read_free( filters[jj].outfile );
        free( filters[jj].filter );
    }

    /* Free remaining memory */
    stk_close( pars->filter_stack);
    stk_close( pars->outfile_stack);

    return(0);
}



/* main routine */
int dmscopy(void)
{
    Parameters *pars;    
    if ( NULL == (pars = get_inputs())) {
        return(-1);
    }

    dmBlock *inBlock;    
    if ( NULL == ( inBlock = dmTableOpen( pars->infile ))) {
        err_msg("ERROR: Problem opening infile '%s'", pars->infile );
        return(-1);
    }

    Filter *filters;
    if ( NULL == ( filters = setup_filters( inBlock, pars ))) {
        return(-1);
    }

    process_infile( inBlock, filters, pars );

    cleanup( filters, pars );
    dmTableClose(inBlock);
    free(pars);
    free(filters);

    return(0);
}


