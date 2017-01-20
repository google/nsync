/* Copyright 2016 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License. */

#ifndef NSYNC_TESTING_HEAP_H_
#define NSYNC_TESTING_HEAP_H_

/* A heap.
   Optionally, elements may have storage for the index to allow deletions from
   arbitrary elements.  A "set" operation sets the field.  Use heap_no_set when
   no field is available.

   Let:
      set (e,i)   sets the index field of the element e to i
      lt (e0, e1) returns whether element e0 < e1

   If
      "a" is an array,
      "n" is is its length,
   then
      To add an element e:
	ensure there are n+1 elements in a[]
	heap_add (a, n, lt, set, e);  // modifies n
      To remove element i:
	heap_remove (a, n, lt, set, i);  // modifies n
      To replace element i with element e:
	heap_adjust (a, n, lt, set, i, e);
*/


#define h_up_(i)   (((i)-1) >> 1)
#define h_down_(i) (((i)<<1) + 1)

#define h_updownall_(up,a,n,i,lt,set,v,s) \
	do { \
		int i_ = (i); \
		int n_ = (n); \
		int j_; \
		if (up) { \
			for (; i_!=0 && ((j_ = h_up_ (i_)), lt ((v), (a)[j_])); i_ = j_) { \
				(a)[i_] = (a)[j_]; \
				set ((a)[i_], i_); \
			} \
		} else { \
			for (; (j_ = h_down_ (i_)) < n_ && ((j_ += (j_+1 < n_ && \
			       lt ((a)[j_+1], (a)[j_]))), lt ((a)[j_], (v))); i_ = j_) { \
				(a)[i_] = (a)[j_]; \
				set ((a)[i_], i_); \
			} \
		} \
		s; \
	} while (0)

#define heap_no_set(a,b) ((void)0)

#define heap_add(a,n,lt,set,v)      h_updownall_ (1,                    (a),      0, \
	(n), lt, set,     (v), ((a)[i_]=(v),     set ((a)[i_], i_), (n)++))
#define heap_remove(a,n,lt,set,i)   h_updownall_ (lt ((a)[n_], (a)[i_]), (a), --(n), \
	(i), lt, set, (a)[n_], ((a)[i_]=(a)[n_], set ((a)[i_], i_)))
#define heap_adjust(a,n,lt,set,i,v) h_updownall_ (lt ((v),     (a)[i_]), (a),   (n), \
	(i), lt, set,     (v), ((a)[i_]=(v),     set ((a)[i_], i_)))

#endif /*NSYNC_TESTING_HEAP_H_*/
