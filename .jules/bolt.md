## 2024-11-03 - Unstaging untracked files
**Learning:** When cleaning up the workspace and unstaging newly added files (files not yet committed), `git reset HEAD <file>` will fail with "pathspec did not match any file(s) known to git" because the files do not exist in HEAD.
**Action:** Use `git rm --cached <file>` or `git restore --staged <file>` to unstage them, and ensure you do this *before* physically deleting them via `rm` to avoid complications.
