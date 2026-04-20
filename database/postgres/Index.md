# Data in Postgres

Consider a logical table,

emp_id |   name   |   dob      |      salary
1          Vivek     15/04/93         15000

Postgres will automatically add a 'row_id' collumn to this. Also sometimes called 'tuple_id'.

Now, the storage subsystem of a DB will store sets of these rows as Pages in a storage device, and reads are done for Pages, not individual rows... ie, say reading first page loads 100 rows onto memory. By default, a page in postgres is 8KB. 

IO operations must be minimized for database performance.

Pages are stored on the Heap.


# Indices in Postgres

An *index* is a set of handles/pointers to data, it is a seperate datastructure from the main table... b-trees are a popular datastructure for indices.

Indices are used to know exactly which page to read on the heap... however indices live on the heap themselves and cost IO to read, hence smaller indices are typically faster.

Tables can be organized in the heap around a index (ie the pages are sequence in the heap in the same sequence as the index)... this is called a 'Clustering Index', and in Postgres this is always the row_id. 


# Cardinality in Indices

B-Trees are good for high cardinality values, like names. Low cardinality fields, like a boolean field might give marginal benefit. 

Even if index is available, postgres may decide to *not* use it, eg low cardinality, too many parrallel indices needed for a query ect... in these cases it can decide to do a Bitmap scan instead.

Bitmap is a index made using inbuilt row_id and other indices that the table has.

## Bitmap scan
Bitmap scans are a fallback method of searching, postgres decides upon it on it's own heuristics. 
eg: CREATE INDEX idx_btree_active ON students USING btree (grades); 
    SELECT NAME FROM grades WHERE id > 95; 

In these sort of conditions, index scans value can be questionable - it takes IO to read the index and jump to value in heap based on index pointer. 

A *bitmap* is built using an index, with bitmap index --> page number, eg bm[0] --> value represents page number 0. 
eg: bm[5] = 1... means there in a row in page 5 of grades index that satisfies id > 95
