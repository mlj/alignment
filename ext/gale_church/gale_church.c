/* NATools - Package with parallel corpora tools
 * Original Implementation Copyright:
 *               Pernilla Danielsson and Daniel Ridings
 * Algorithm Copyright:
 *               William Gale and Kenneth Church
 * Copyright (C) 2002-2004  Alberto Simões

 * Ruby adaptation:
 *   Copyright (C) 2008  Marius L. Jøhndal
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "ruby.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

VALUE mLogos = Qnil;
VALUE mGaleChurch = Qnil;

void Init_gale_church();

VALUE method_align(VALUE self, VALUE len1, VALUE len2);

struct alignment {
  int x1;
  int y1;
  int x2;
  int y2;
  int d;
};

static int two_side_distance(int x1, int y1, int x2, int y2);
static int seq_align(int *x, int *y, int nx, int ny, int (*dist_funct)(),
                     struct alignment **align);

void Init_gale_church()
{
  mLogos = rb_define_module("Alignment");
  mGaleChurch = rb_define_module_under(mLogos, "GaleChurch");
  rb_define_module_function(mGaleChurch, "align", method_align, 2);
}

VALUE method_align(VALUE self, VALUE len1, VALUE len2)
{
  VALUE ret, res1, res2, grouped_regions1, grouped_regions2;
  
  int n, i, ix, iy, prevx, prevy;
  struct alignment *align, *a;
  int *len1_x;
  int *len2_x;

  res1 = rb_ary_new();
  res2 = rb_ary_new();

  len1_x = (int *)malloc(RARRAY(len1)->len * sizeof(int));
  len2_x = (int *)malloc(RARRAY(len2)->len * sizeof(int));
  
  for (i = 0; i < RARRAY(len1)->len; i++)
    len1_x[i] = NUM2INT(rb_ary_entry(len1, i));
  
  for (i = 0; i < RARRAY(len2)->len; i++)
    len2_x[i] = NUM2INT(rb_ary_entry(len2, i));
  
  n = seq_align(len1_x, len2_x, RARRAY(len1)->len, RARRAY(len2)->len,
                two_side_distance, &align);
  
  free(len1_x);
  free(len2_x);

  prevx = prevy = ix = iy = 0;
  
  for (i = 0; i < n; i++) {
    a = &align[i];
    
    if (a->x2 > 0) 
      ix++; 
    else if (a->x1 == 0) 
      ix--;
    
    if (a->y2 > 0) 
      iy++; 
    else if (a->y1 == 0) 
      iy--;
    
    if (a->x1 == 0 && a->y1 == 0 && a->x2 == 0 && a->y2 == 0) {
      ix++; 
      iy++;
    }
    
    ix++;
    iy++;
    
    grouped_regions1 = rb_ary_new();
    for (; prevx < ix; prevx++)
      rb_ary_push(grouped_regions1, INT2FIX(prevx));
    rb_ary_push(res1, grouped_regions1);    
    
    grouped_regions2 = rb_ary_new();
    for (; prevy < iy; prevy++)
      rb_ary_push(grouped_regions2, INT2FIX(prevy));
    rb_ary_push(res2, grouped_regions2);    
  }
  
  free(align);

  ret = rb_ary_new();
  rb_ary_push(ret, res1);
  rb_ary_push(ret, res2);

  return ret;
}

/********************************************************************/

/**
 * @file
 * @brief Sentence-aligner main program
 *
 * Sentence-aligner software heavily based on Pernilla Danielsson and
 * Daniel Ridings implementation of William Gale and Kenneth Church
 * sentence aligner algorithm
 *
 * The compiled program is used as
 * <pre>
 *  nat-sentence-aligner -D '.PARA' -d '.End of Sentence'  file1 file2
 * </pre>
 * where both <i>file1</i> and <i>file2</i> are tokenized and with a 
 * token per line. In the example, '.PARA' is the hard delimiter,
 * and '.End of Sentence' the soft delimiter. They are considered as
 * single tokens, and as such, should appear in a line by themselves.
 *
 * The program is allowed to delete soft delimiters as necessary in
 * order to align the files, but it cannot change hard delimiters.
 *
 * The output will be written in two files: file1.al and file2.al
 * where each one contains aligned sentences.
 *
 * For debugging it can be useful to output the data in just one file.
 * In this case, use the '-s' switch and just the 'file1.al' will be
 * created.
 *
 * @todo Check if we want to document all the functions
 */


#define dist(x,y) distances[(x) * ((ny) + 1) + (y)]
#define pathx(x,y) path_x[(x) * ((ny) + 1) + (y)]
#define pathy(x,y) path_y[(x) * ((ny) + 1) + (y)]

/**
 * @brief Contant representing a big align distance between two
 * sentences
 */
#define BIG_DISTANCE 2500

/* 

seq_align by Mike Riley
Sequence alignment routine.
This version allows for contraction/expansions.

x and y are sequences of objects, represented as non-zero ints, to be aligned.

dist_funct(x1, y1, x2, y2) is a distance function of 4 args:

dist_funct(x1, y1, 0, 0) gives cost of substitution of x1 by y1.
dist_funct(x1, 0, 0, 0) gives cost of deletion of x1.
dist_funct(0, y1, 0, 0) gives cost of insertion of y1.
dist_funct(x1, y1, x2, 0) gives cost of contraction of (x1,x2) to y1.
dist_funct(x1, y1, 0, y2) gives cost of expansion of x1 to (y1,y2).
dist_funct(x1, y1, x2, y2) gives cost to match (x1,x2) to (y1,y2).

align is the alignment, with (align[i].x1, align[i].x2) aligned
with (align[i].y1, align[i].y2).  Zero in align[].x1 and align[].y1
correspond to insertion and deletion, respectively.  Non-zero in
align[].x2 and align[].y2 correspond to contraction and expansion,
respectively.  align[].d gives the distance for that pairing.

The function returns the length of the alignment.

*/

static int seq_align(int *x, int *y, int nx, int ny, int (*dist_funct)(),
    struct alignment **align)
{
  int *distances, *path_x, *path_y, n;
  int i, j, oi, oj, di, dj, d1, d2, d3, d4, d5, d6, dmin;
  struct alignment *ralign;
  
  distances = (int *) malloc((nx + 1) * (ny + 1) * sizeof(int));
  path_x = (int *) malloc((nx + 1) * (ny + 1) * sizeof(int));
  path_y = (int *) malloc((nx + 1) * (ny + 1) * sizeof(int));
  ralign = (struct alignment *) malloc((nx + ny) * sizeof(struct alignment));
  
  for (j = 0; j <= ny; j++) {
    for (i = 0; i <= nx; i++) {
      d1 = i>0 && j>0 ?		/* substitution */
	dist(i-1, j-1) + (*dist_funct)(x[i-1], y[j-1], 0, 0)
	: INT_MAX;
      d2 = i>0 ?		/* deletion */
	dist(i-1, j) + (*dist_funct)(x[i-1], 0, 0, 0)
	: INT_MAX;
      d3 = j>0 ?		/* insertion */
	dist(i, j-1) + (*dist_funct)(0, y[j-1], 0, 0)
	: INT_MAX;
      d4 = i>1 && j>0 ?		/* contraction */
	dist(i-2, j-1) + (*dist_funct)(x[i-2], y[j-1], x[i-1], 0)
	: INT_MAX;
      d5 = i>0 && j>1 ?		/* expansion */
	dist(i-1, j-2) + (*dist_funct)(x[i-1], y[j-2], 0, y[j-1])
	: INT_MAX;
      d6 = i>1 && j>1 ?		/* melding */
	dist(i-2, j-2) + (*dist_funct)(x[i-2], y[j-2], x[i-1], y[j-1])
	: INT_MAX;
      
      dmin = d1;
      if (d2<dmin) dmin=d2;
      if (d3<dmin) dmin=d3;
      if (d4<dmin) dmin=d4;
      if (d5<dmin) dmin=d5;
      if (d6<dmin) dmin=d6;
      
      if (dmin == INT_MAX) {
	dist(i,j) = 0;
      }
      else if (dmin == d1) {
	dist(i,j) = d1;
	pathx(i,j) = i-1;
	pathy(i,j) = j-1;
      }
      else if (dmin == d2) {
	dist(i,j) = d2;
	pathx(i,j) = i-1;
	pathy(i,j) = j;
      }
      else if (dmin == d3) {
	dist(i,j) = d3;
	pathx(i,j) = i;
	pathy(i,j) = j-1;
      }
      else if (dmin == d4) {
	dist(i,j) = d4;
	pathx(i,j) = i-2;
	pathy(i,j) = j-1;
      }
      else if (dmin == d5){
	dist(i,j) = d5;
	pathx(i,j) = i-1;
	pathy(i,j) = j-2;
      }
      else			/* dmin == d6 */ {
	dist(i,j) = d6;
	pathx(i,j) = i-2;
	pathy(i,j) = j-2;
      }
    }  
  }
  
  n = 0;
  for (i=nx, j=ny ; i>0 || j>0 ; i = oi, j = oj) {
    oi = pathx(i, j);
    oj = pathy(i, j);
    di = i - oi;
    dj = j - oj;
    
    if (di == 1 && dj == 1) {  /* substitution */
      ralign[n].x1 = x[i-1];
      ralign[n].y1 = y[j-1];
      ralign[n].x2 = 0;
      ralign[n].y2 = 0;
      ralign[n++].d = dist(i, j) - dist(i-1, j-1);
    }
    
    else if (di == 1 && dj == 0) {  /* deletion */
      ralign[n].x1 = x[i-1];
      ralign[n].y1 = 0;
      ralign[n].x2 = 0;
      ralign[n].y2 = 0;
      ralign[n++].d = dist(i, j) - dist(i-1, j);
    }
    
    else if (di == 0 && dj == 1) {  /* insertion */
      ralign[n].x1 = 0;
      ralign[n].y1 = y[j-1];
      ralign[n].x2 = 0;
      ralign[n].y2 = 0;
      ralign[n++].d = dist(i, j) - dist(i, j-1);
    }
    
    else if (dj == 1) {  /* contraction */
      ralign[n].x1 = x[i-2];
      ralign[n].y1 = y[j-1];
      ralign[n].x2 = x[i-1];
      ralign[n].y2 = 0;
      ralign[n++].d = dist(i, j) - dist(i-2, j-1);
    }
    
    else if (di == 1) {    /* expansion */
      ralign[n].x1 = x[i-1];
      ralign[n].y1 = y[j-2];
      ralign[n].x2 = 0;
      ralign[n].y2 = y[j-1];
      ralign[n++].d = dist(i, j) - dist(i-1, j-2);
    }
    else /* di == 2 && dj == 2 */ { /* melding */
      ralign[n].x1 = x[i-2];
      ralign[n].y1 = y[j-2];
      ralign[n].x2 = x[i-1];
      ralign[n].y2 = y[j-1];
      ralign[n++].d = dist(i, j) - dist(i-2, j-2);
    }
  }
  
  *align = (struct alignment *) malloc(n * sizeof(struct alignment));
  
  for (i=0; i<n; i++)
    bcopy(ralign + i, (*align) + (n-i-1), sizeof(struct alignment));
  
  free(distances);
  free(path_x);
  free(path_y);
  free(ralign);
  return(n);
}

/* Returns the area under a normal distribution
   from -inf to z standard deviations */
static double pnorm(double z)
{
  double t, pd;
  t = 1/(1 + 0.2316419 * z);
  pd = 1 - 0.3989423 *   
    exp(-z * z/2) * 
    ((((1.330274429 * t - 1.821255978) * t 
       + 1.781477937) * t - 0.356563782) * t + 0.319381530) * t;
  /* see Gradsteyn & Rhyzik, 26.2.17 p932 */
  return(pd);
}


/* Return -100 * log probability that an English sentence of length
   len1 is a translation of a foreign sentence of length len2.  The
   probability is based on two parameters, the mean and variance of
   number of foreign characters per English character.
*/

static int match(int len1,int len2)
{
  double z, pd, mean;
  
  /* foreign characters per english character */
  double foreign_chars_per_eng_char = 1;
  
  /* variance per english character */
  double var_per_eng_char = 6.8 ;	
  
  if (len1==0 && len2==0) return(0);
  mean = (len1 + len2/foreign_chars_per_eng_char)/2;
  z = (foreign_chars_per_eng_char * len1 - len2)/sqrt(var_per_eng_char * mean);
  
  /* Need to deal with both sides of the normal distribution */
  if (z < 0) z = -z;
  pd = 2 * (1 - pnorm(z));
  
  if (pd > 0) return((int)(-100 * log(pd)));
  else return(BIG_DISTANCE);
}

static int two_side_distance(int x1, int y1, int x2, int y2)
{
  int penalty21 = 230;		/* -100 * log([prob of 2-1 match] / [prob of 1-1 match]) */
  int penalty22 = 440;		/* -100 * log([prob of 2-2 match] / [prob of 1-1 match]) */
  int penalty01 = 450;		/* -100 * log([prob of 0-1 match] / [prob of 1-1 match]) */
  
  if (x2 == 0 && y2 == 0)
    if (x1 == 0)			/* insertion */
      return(match(x1, y1) + penalty01);
    else if (y1 == 0)		/* deletion */
      return(match(x1, y1) + penalty01);
    else
      return (match(x1, y1)); /* substitution */
    else if (x2 == 0)		/* expansion */
      return (match(x1, y1 + y2) + penalty21);
    else if (y2 == 0)		/* contraction */
      return(match(x1 + x2, y1) + penalty21); 
    else				/* melding */
      return(match(x1 + x2, y1 + y2) + penalty22);
}
