# Full Text Search in PostGres

## Core Concepts
**Documents and Tokens**
PostgreSQL converts text into a tsvector — a sorted list of distinct lexemes (normalized word forms). "Running quickly" becomes 'quick':2 'run':1, where numbers indicate positions.

**Search Queries**
You express searches as tsquery objects that support boolean operators: 'cat & dog' (both words), 'cat | dog' (either), 'cat & !dog' (cat but not dog), and phrase searches like 'quick <-> brown' (adjacent words).

**Text Processing**
The key is normalization. PostgreSQL removes suffixes (runs → run), strips stop words (the, and, is), and handles case-insensitivity. Different languages get different rules — English uses the Snowball stemmer by default.

## Example

In test_full_text in the bash script, see the CREATE TABLE command,
```
CREATE TABLE user_summaries (
        userid INT PRIMARY KEY,
        summary VARCHAR(255),
        summary_tsv tsvector GENERATED ALWAYS AS (to_tsvector('english', summary)) STORED
    );
```

summary_tsv is a collumn generated from summary  and is a ts_vector. (The STORED keyword means actually store, not discard as temporary data) 


Full text queries then use ts_query to query the ts_vector,
```SELECT userid FROM user_summaries WHERE summary_tsv @@ to_tsquery('english','engineer') ORDER BY userid;```
