# (Follow-up changes after code modification)

#!/bin/bash

# Refresh compile_commands.json
./mkbuild.sh

# Refresh tags
cd lib/src/ && ctags -R . && cd $OLDPWD

# Commit changes
git add compile_commands.json lib/src/tags && git commit -m "Autocommit"
