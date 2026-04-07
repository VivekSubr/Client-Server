* Typically databases assign a hidden 'row_id' to each row.

* Databases have a storage system which stores rows in logical 'Pages'. Every read operation which read data Page-by-Page. Pages are a constant sized logical unit, eg: they are 8KB in PostGres. 


# Index
An index is a seperate structure from the main data which lives on the heap, and has handles/pointers to data on the heap. 

The index is created from collumns in the table and is used to quickly know which Page is needed to be read, preventing wasted IO.

B-Trees are a common choice of index data-structure.

Indices can be clustered or non-clustered. Clustered index --> The physical order of data rows follows this logical index... non-cluster means it does not. 

NOTE - rows do *not* have any order in the heap, unless table has a Primary Key. Primary Keys cause 'clustering'... but they are *not* an index, they are part of the heap itself! Secondary Keys are non-clustering indices - ie exist as a seperate struct.
(This is *not* true for postgres - there both primary and secondary keys are just non-cluster indices, and it has a seperate CLUSTER command that re-orgs the heap)


# Row vs Column Orientation

In Row oriented, getting a row --> get all collumns for that row. Hence it might take multiple IOs to get the page with required row, but then have all the collumns for it.

But say you want to sum elements of a collumn, eg: Sum(Salary) from employee table -> this *needs* multiple IO here, but in a Collumn oriented DB it'll need far less.

But in Collumn oriented DB -> getting all details for an employee -> expensive query, but easy in row oriented.