import sqlite3
SQL_DATABASE_NAME = "obsw_mib.db"


class SqlWriter:
    def __init__(self, filename: str = SQL_DATABASE_NAME):
        self.filename = filename
        self.conn = sqlite3.connect(self.filename)

    def open(self, sql_creation_command: str):
        print("SQL Writer: Opening " + self.filename)
        self.conn.execute(sql_creation_command)

    def delete(self, sql_deletion_command):
        print("SQL Writer: Deleting SQL table")
        self.conn.execute(sql_deletion_command)

    def write_entries(self, sql_insertion_command, current_entry):
        cur = self.conn.cursor()
        cur.execute(sql_insertion_command, current_entry)
        return cur.lastrowid

    def commit(self):
        print("SQL Writer: Commiting SQL table")
        self.conn.commit()

    def close(self):
        self.conn.close()

    def sql_writing_helper(self, creation_cmd, insertion_cmd, mib_table: dict, deletion_cmd: str=""):
        if deletion_cmd != "":
            self.delete(deletion_cmd)
        self.open(creation_cmd)
        for i in mib_table:
            self.write_entries(insertion_cmd, mib_table[i])
        self.commit()
        self.close()
