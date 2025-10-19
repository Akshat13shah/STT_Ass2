
import re
import sys
import argparse
import os
import subprocess
from collections import defaultdict, OrderedDict


def remove_block_comments(code: str) -> str:
    # Remove /* ... */ (non-greedy)
    return re.sub(r"/\*.*?\*/", "", code, flags=re.S)


def remove_line_comments(code: str) -> str:
    # Remove // comments
    return re.sub(r"//.*$", "", code, flags=re.M)


def preprocess(code: str) -> list:
    code = remove_block_comments(code)
    code = remove_line_comments(code)
    # Split into lines and keep original indentation-trimmed form
    lines = [ln.rstrip() for ln in code.splitlines()]
    # Keep empty lines (we will ignore them later for leaders but keep indexing stable)
    return lines


def next_non_empty(lines, idx):
    i = idx + 1
    while i < len(lines) and lines[i].strip() == "":
        i += 1
    return i if i < len(lines) else None


def find_matching_brace(lines, start_idx):
    """
    Given lines and an index start_idx, find the line index of the matching closing brace '}'
    assuming the opening brace '{' is somewhere on or after start_idx.
    Returns the index of the line containing the matching '}', or None if not found.
    """
    balance = 0
    found_any = False
    for i in range(start_idx, len(lines)):
        ln = lines[i]
        # Count braces
        for ch in ln:
            if ch == '{':
                balance += 1
                found_any = True
            elif ch == '}':
                balance -= 1
        if found_any and balance == 0:
            return i
    return None


def find_leaders_and_controls(lines):
    """
    Heuristic detection of leaders and control-structure bookkeeping.
    Returns: leaders_set, control_structures (list of dicts describing if/loop blocks)
    Each control structure dict contains:
      - type: 'if'|'for'|'while'|'do'
      - cond_line: index where 'if'/'for' etc occurs
      - then_start, then_end (line indices)
      - else_start, else_end (may be None)
      - join (index of line after the whole construct, may be None)
    """
    leaders = set()
    controls = []
    n = len(lines)

    # Rule: first instruction is a leader -> find first non-empty
    first = None
    for i, ln in enumerate(lines):
        if ln.strip() != "":
            first = i
            leaders.add(i)
            break

    i = 0
    while i < n:
        ln = lines[i].strip()
        if ln == "":
            i += 1
            continue

        # Detect 'if' / 'for' / 'while' / 'do' at start of line (heuristic)
        if re.match(r'^(if|for|while)\b', ln):
            typ = re.match(r'^(if|for|while)\b', ln).group(1)
            cond_line = i

            # Find start of body: look for '{' from the condition line onwards
            body_open_line = None
            for j in range(i, min(n, i+50)):    
                if '{' in lines[j]:
                    body_open_line = j
                    break
                # if we reach a semicolon on the same line and no '{', it's a single-statement body
                if ';' in lines[j] and j > i:
                    break

            if body_open_line is not None:
                then_start = next_non_empty(lines, body_open_line)
                if then_start is None:
                    then_start = body_open_line
                then_end = find_matching_brace(lines, body_open_line)
                if then_end is None:
                    then_end = body_open_line
            else:
                # Single-statement: take next non-empty line
                then_start = next_non_empty(lines, i)
                then_end = then_start

            # Look for 'else' after then_end
            else_start = None
            else_end = None
            join = None
            after_then = None
            if then_end is not None:
                after_then = next_non_empty(lines, then_end)
            if after_then is not None and lines[after_then].strip().startswith('else'):
                # process else
                k = after_then
                # If 'else if' -> we'll treat else as another if (nested)
                # find body as with if
                body_open_else = None
                for j in range(k, min(n, k+50)):
                    if '{' in lines[j]:
                        body_open_else = j
                        break
                    if ';' in lines[j] and j > k:
                        break
                if body_open_else is not None:
                    else_start = next_non_empty(lines, body_open_else)
                    else_end = find_matching_brace(lines, body_open_else)
                    if else_end is None:
                        else_end = body_open_else
                    join = next_non_empty(lines, else_end)
                else:
                    else_start = next_non_empty(lines, k)
                    else_end = else_start
                    join = next_non_empty(lines, else_end)
            else:
                # No else: join is the next non-empty after then_end
                join = after_then

            # Mark leaders according to the lab rules
            leaders.add(cond_line)
            if then_start is not None:
                leaders.add(then_start)
            if else_start is not None:
                leaders.add(else_start)
            if join is not None:
                leaders.add(join)

            controls.append({
                'type': typ,
                'cond_line': cond_line,
                'then_start': then_start,
                'then_end': then_end,
                'else_start': else_start,
                'else_end': else_end,
                'join': join
            })

            # Advance i past the construct
            if else_end is not None:
                i = else_end + 1
            elif then_end is not None:
                i = then_end + 1
            else:
                i += 1
            continue

        # detect do/while: 'do' starts then 'while' after
        if re.match(r'^do\b', ln):
            typ = 'do'
            cond_line = i
            # find matching 'while' after a closing brace or semicolon
            # find the index of the line which contains 'while' that closes the do-while
            # simple heuristic: search next 50 lines for a line starting with 'while' or containing ') ;'
            end_line = None
            for j in range(i, min(n, i+100)):
                if re.match(r'^while\b', lines[j].strip()) or (')' in lines[j] and ';' in lines[j]):
                    end_line = j
                    break
            then_start = next_non_empty(lines, i)
            then_end = end_line
            join = next_non_empty(lines, end_line) if end_line is not None else None
            leaders.add(i)
            if then_start is not None:
                leaders.add(then_start)
            if join is not None:
                leaders.add(join)
            controls.append({
                'type': 'do',
                'cond_line': cond_line,
                'then_start': then_start,
                'then_end': then_end,
                'else_start': None,
                'else_end': None,
                'join': join
            })
            i = (then_end or i) + 1
            continue

        # Also: any instruction that comes immediately after a branch/jump/loop is a leader.
        # We partly covered that with 'join' detection above. To be conservative, if line includes 'return' or 'goto' or 'break' or 'continue', mark next non-empty as leader.
        if re.search(r'\b(return|goto|break|continue)\b', ln):
            nx = next_non_empty(lines, i)
            if nx is not None:
                leaders.add(nx)

        i += 1

    # final adjustment: ensure leaders are sorted and within range
    leaders = set([i for i in leaders if i is not None and 0 <= i < n and lines[i].strip() != ""])
    return leaders, controls


def build_basic_blocks(lines, leaders_set):
    leaders = sorted(list(leaders_set))
    blocks = OrderedDict()
    # map start_line -> block_id
    start_to_bid = {}
    for idx, s in enumerate(leaders):
        bid = f'B{idx}'
        start_to_bid[s] = bid
    # Determine block ranges: each leader spans until just before next leader
    for idx, s in enumerate(leaders):
        end = leaders[idx+1]-1 if idx+1 < len(leaders) else len(lines)-1
        # Trim leading/trailing blank lines
        block_lines = []
        for i in range(s, end+1):
            if lines[i].strip() != "":
                block_lines.append((i, lines[i]))
        blocks[start_to_bid[s]] = {
            'start': s,
            'end': end,
            'lines': block_lines
        }
    return blocks, start_to_bid


def find_block_by_line(start_to_bid, line_idx):
    # find block whose starting line equals line_idx
    return start_to_bid.get(line_idx, None)


def build_cfg_edges(blocks, start_to_bid, controls):
    # edges: list of (src_bid, dst_bid, label)
    edges = []
    # Create a quick mapping from start_line to bid
    start_line_to_bid = {info['start']: bid for bid, info in blocks.items()}

    # Build a lookup for controls keyed by cond_line
    control_by_cond = {c['cond_line']: c for c in controls}

    # Build an ordered list of start_lines so that we can add sequential edges
    starts = sorted(start_line_to_bid.keys())

    # Map start line to index in starts
    start_index = {s: idx for idx, s in enumerate(starts)}

    for bid, info in blocks.items():
        s = info['start']
        e = info['end']
        # If this block is a condition block (one of our control cond_lines)
        if s in control_by_cond:
            c = control_by_cond[s]
            # True branch -> then_start
            if c['then_start'] is not None:
                th_bid = start_line_to_bid.get(c['then_start'])
                if th_bid:
                    edges.append((bid, th_bid, 'true'))
            # False branch -> else_start if present else join
            if c['else_start'] is not None:
                el_bid = start_line_to_bid.get(c['else_start'])
                if el_bid:
                    edges.append((bid, el_bid, 'false'))
                # connect both branches to join later
            else:
                # false edge to join
                if c['join'] is not None:
                    j_bid = start_line_to_bid.get(c['join'])
                    if j_bid:
                        edges.append((bid, j_bid, 'false'))
            # Additionally, if join exists, ensure then and else connect to join
            if c['join'] is not None:
                # find block ids for ends of then and else and connect to join where appropriate
                join_bid = start_line_to_bid.get(c['join'])
                # then_end block id: find the block starting at or right after then_end
                if c['then_end'] is not None:
                    # the statement after then_end is the join, but we will add sequential edges below
                    pass
            # For loops, add back edges from body to cond
            if c['type'] in ('for', 'while', 'do'):
                if c['then_start'] is not None:
                    body_bid = start_line_to_bid.get(c['then_start'])
                    if body_bid:
                        edges.append((body_bid, bid, 'back'))
                # Condition -> exit (join)
                if c['join'] is not None:
                    j_bid = start_line_to_bid.get(c['join'])
                    if j_bid:
                        edges.append((bid, j_bid, 'exit'))
        else:
            # Not an explicit condition block -> sequential edge to next block if any
            # Find next block in starts after s
            idx = start_index.get(s)
            if idx is not None and idx+1 < len(starts):
                next_start = starts[idx+1]
                next_bid = start_line_to_bid.get(next_start)
                if next_bid:
                    edges.append((bid, next_bid, 'seq'))

    # Additional: ensure that blocks that represent bodies have edges to join if identified in controls
    # For each control structure, connect the last block inside then to join, and last block inside else to join
    for c in controls:
        join = c.get('join')
        if join is None:
            continue
        join_bid = start_line_to_bid.get(join)
        if c.get('then_end') is not None:
            # find block that contains then_end (block with start <= then_end and end >= then_end)
            for bid, info in blocks.items():
                if info['start'] <= c['then_end'] <= info['end']:
                    if join_bid and (bid, join_bid, '') not in edges:
                        edges.append((bid, join_bid, 'seq'))
                    break
        if c.get('else_end') is not None:
            for bid, info in blocks.items():
                if info['start'] <= c['else_end'] <= info['end']:
                    if join_bid and (bid, join_bid, '') not in edges:
                        edges.append((bid, join_bid, 'seq'))
                    break
    # Deduplicate edges
    seen = set()
    uniq = []
    for a,b,l in edges:
        key = (a,b,l)
        if key not in seen:
            seen.add(key)
            uniq.append((a,b,l))
    return uniq


def write_dot(blocks, edges, outdot):
    with open(outdot, 'w') as f:
        f.write('digraph CFG {\n')
        f.write('  node [shape=box, fontname="monospace"];\n')
        for bid, info in blocks.items():
            label_lines = [f'{bid}:']
            if info['lines']:
                for (ln_idx, ln) in info['lines']:
                    # escape quotes and backslashes for DOT
                    esc = ln.replace('\\', '\\\\').replace('"', '\\"')
                    label_lines.append(f'{ln_idx}: {esc}')
            else:
                # Block has no lines, put "void"
                label_lines.append('void')
            label = '\\n'.join(label_lines)
            f.write(f'  {bid} [label="{label}"];\n')
        for src, dst, lab in edges:
            if lab and lab != 'seq':
                f.write(f'  {src} -> {dst} [label="{lab}"];\n')
            else:
                f.write(f'  {src} -> {dst};\n')
        f.write('}\n')
    print(f'Wrote DOT to {outdot}')



def render_dot(dotfile, pngfile):
    try:
        subprocess.run(['dot', '-Tpng', dotfile, '-o', pngfile], check=True)
        print(f'Rendered PNG to {pngfile}')
    except Exception as e:
        print('Could not render PNG automatically. Make sure Graphviz "dot" is installed and on PATH.')
        print('You can manually run: dot -Tpng', dotfile, '-o', pngfile)


# ---------- Definitions and dataflow ----------

def find_definitions(blocks):
    """Find assignment definitions in blocks. Return defs dict and var->defs map."""
    defs = OrderedDict()
    var_map = defaultdict(list)
    counter = 1
    # Regex to find assignment ops but avoid '=='
    assign_re = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*(?:\+=|-=|\*=|/=|%=|<<=|>>=|&=|\|=|\^=|=)\s*")
    for bid, info in blocks.items():
        for (ln_idx, ln) in info['lines']:
            # Ignore lines that look like conditional checks containing '=' (e.g., ==) handled by regex
            m = assign_re.search(ln)
            if m:
                var = m.group(1)
                did = f'D{counter}'
                defs[did] = {
                    'var': var,
                    'block': bid,
                    'line_idx': ln_idx,
                    'text': ln.strip()
                }
                var_map[var].append(did)
                counter += 1
    return defs, var_map


def compute_gen_kill(blocks, defs, var_map):
    gen = defaultdict(set)
    kill = defaultdict(set)
    # For each block: gen = defs in block; kill = all definitions of those variables that are not in this block
    for did, meta in defs.items():
        bid = meta['block']
        gen[bid].add(did)
    for bid in blocks.keys():
        # for each variable defined in gen[bid], kill all other defs of that variable
        killed = set()
        for d in gen[bid]:
            var = defs[d]['var']
            for other in var_map[var]:
                if other != d:
                    killed.add(other)
        kill[bid] = killed
    return gen, kill


def reaching_definitions_worklist(blocks, edges, gen, kill):
    # Build predecessor sets
    preds = defaultdict(set)
    for src, dst, lab in edges:
        preds[dst].add(src)
    # Initialize
    in_sets = {bid: set() for bid in blocks}
    out_sets = {bid: set() for bid in blocks}

    changed = True
    iteration = 0
    history = []
    while changed:
        iteration += 1
        changed = False
        snapshot = {'iter': iteration, 'in': {}, 'out': {}}
        for bid in blocks:
            # in[B] = union of out[p] for p in preds[B]
            new_in = set()
            for p in preds[bid]:
                new_in |= out_sets[p]
            # out[B] = gen[B] U (in[B] - kill[B])
            new_out = set(gen.get(bid, set())) | (new_in - kill.get(bid, set()))
            if new_in != in_sets[bid] or new_out != out_sets[bid]:
                changed = True
            in_sets[bid] = new_in
            out_sets[bid] = new_out
            snapshot['in'][bid] = set(new_in)
            snapshot['out'][bid] = set(new_out)
        history.append(snapshot)
        # safety cap (shouldn't be needed but avoids infinite loops on weird input)
        if iteration > 200:
            print('Warning: reached 200 iterations; stopping')
            break
    return in_sets, out_sets, history


def write_report(outpath, defs, var_map, gen, kill, blocks, in_sets, out_sets, history):
    with open(outpath, 'w') as f:
        f.write('=== Definitions (ID -> var, block, line) ===\n')
        for did, m in defs.items():
            f.write(f"{did}: {m['var']} in {m['block']} (line {m['line_idx']}): {m['text']}\n")
        f.write('\n=== gen[B] and kill[B] ===\n')
        for bid in blocks.keys():
            f.write(f"{bid}: gen={sorted(list(gen.get(bid, [])))} kill={sorted(list(kill.get(bid, [])))}\n")
        f.write('\n=== Dataflow iterations (snapshot per iteration) ===\n')
        for snap in history:
            f.write(f"-- Iteration {snap['iter']} --\n")
            for bid in blocks.keys():
                f.write(f"{bid}: in={sorted(list(snap['in'][bid]))} out={sorted(list(snap['out'][bid]))}\n")
            f.write('\n')
        f.write('\n=== Final in[B] / out[B] ===\n')
        for bid in blocks.keys():
            f.write(f"{bid}: in={sorted(list(in_sets[bid]))} out={sorted(list(out_sets[bid]))}\n")
    print(f'Wrote reaching definitions report to {outpath}')


# ----------------- Utilities -----------------

def compute_metrics(blocks, edges):
    N = len(blocks)
    E = len(edges)
    CC = E - N + 2
    return N, E, CC


# ----------------- Main -----------------

def analyze_c_file(cpath, render=False):
    with open(cpath, 'r') as f:
        code = f.read()
    lines = preprocess(code)
    leaders, controls = find_leaders_and_controls(lines)
    blocks, start_to_bid = build_basic_blocks(lines, leaders)
    edges = build_cfg_edges(blocks, start_to_bid, controls)
    base = os.path.splitext(os.path.basename(cpath))[0]
    outdot = base + '_cfg.dot'
    write_dot(blocks, edges, outdot)
    if render:
        pngfile = base + '_cfg.png'
        render_dot(outdot, pngfile)

    N, E, CC = compute_metrics(blocks, edges)
    print('Metrics: N (nodes)=', N, 'E (edges)=', E, 'Cyclomatic Complexity =', CC)

    defs, vmap = find_definitions(blocks)
    gen, kill = compute_gen_kill(blocks, defs, vmap)
    in_sets, out_sets, history = reaching_definitions_worklist(blocks, edges, gen, kill)
    outrep = base + '_reaching.txt'
    write_report(outrep, defs, vmap, gen, kill, blocks, in_sets, out_sets, history)
    print('\nDone.\n')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='CFG + Reaching Definitions lightweight tool')
    parser.add_argument('cfile', help='Path to a single .c file (standalone)')
    parser.add_argument('--render', action='store_true', help='Attempt to render DOT to PNG using dot')
    args = parser.parse_args()
    analyze_c_file(args.cfile, render=args.render)
