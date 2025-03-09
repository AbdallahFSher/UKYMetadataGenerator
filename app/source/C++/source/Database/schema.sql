CREATE TABLE objects (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    type TEXT NOT NULL CHECK (type IN ('Grant', 'Project', 'Specific Aim', 'Study', 'Focus', 'Assay', 'Data')),
    parent_id INTEGER,
    FOREIGN KEY (parent_id) REFERENCES objects(id)
);
