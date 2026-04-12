# 1. Find and delete code files that don't have a matching ans file
for i in {1..900}; do
    if [[ -f "code$i.txt" ]] && [[ ! -f "ans$i.txt" ]]; then
        rm "code$i.txt"
    fi
done

# 2. Re-number both sets of files sequentially
count=1
for i in {1..900}; do
    if [[ -f "code$i.txt" ]] && [[ -f "ans$i.txt" ]]; then
        # Only rename if the number actually needs to change
        if [[ $i -ne $count ]]; then
            mv "code$i.txt" "code$count.txt"
            mv "ans$i.txt" "ans$count.txt"
        fi
        ((count++))
    fi
done