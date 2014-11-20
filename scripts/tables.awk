#function parse_table_header(name2col_hash, col2name_hash,     i) {
function parse_table_header(name2col_hash,      i) {
#	getline; 
	for(i=1; i<=NF; i++) {
		name2col_hash[$i] = i;
#		col2name_hash[i] = $i;
	}
}

function get_skip_column_indexes(skip_columns_str, name2col_hash, skip_columns_list,    a, i) {
	split(skip_columns_str, a, " "); 
	for(i in a) {
		if (!(a[i] in name2col_hash)) {
			print "ERROR: column \""a[i]"\" was not found in table header!" >> "/dev/stderr";
			exit 1;
		}
		skip_columns_list[name2col_hash[a[i]]] = 1;
	} 
}

function skip_columns(skip_columns_list,    separator, i) {
	res = "";
	separator = "";
	for (i=1; i<=NF; i++) {
		if (!( i in skip_columns_list)) {
			res = res""sprintf("%s%s", separator, $i);
			separator = " ";
		}
	}
	return res;
}

function move_column(column_orig_pos, column_new_pos,    tmp) {
#	tmp = $column_new_pos;
#	$column_new_pos = $column_orig_pos;
#	$column_orig_pos = tmp;

	if (NEW_POSITION < col[COLUMN_NAME])
	{
		tmp = $col[COLUMN_NAME];
		for(i=col[COLUMN_NAME]; i>NEW_POSITION; i--) {
			$i = $(i-1);
		}
		$NEW_POSITION = tmp;
	} 
	else if (NEW_POSITION > col[COLUMN_NAME])
	{
		tmp = $col[COLUMN_NAME];
		for(i=col[COLUMN_NAME]; i<NEW_POSITION; i++) {
			$i = $(i+1);
		}
		$NEW_POSITION = tmp;
	}

#	print;
#	printf "\n";
}
