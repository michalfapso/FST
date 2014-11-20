# Usage:
#
# 1:
#	GREETING=Hello NAME=world ./script.sh
#
#	. ~/bin/args_lib.sh
#	required_vars GREETING NAME
#	echo "$GREETING, $NAME!"
#
# 2:
#	./script.sh -greeting Hello -name world
#
#	. ~/bin/args_lib.sh
#	required_vars greeting name
#	echo "$greeting, $name!"

function required_vars
{
	exec_name=$0
	missing_vars=""
	missing_vars_sep=""
	for var_name in $@; do
		if [ -z "`set | grep "^$var_name="`" ]; then
			missing_vars="$missing_vars$missing_vars_sep$var_name"
			missing_vars_sep=", "
		fi
	done
	if [ ! -z "$missing_vars" ]; then
		echo "ERROR: Some variables are not set! ($missing_vars)" >&2
		echo "Usage: `echo $@ | awk '{for(i=1;i<=NF;i++){printf $i"=... "}}'`$exec_name" >&2
		exit 1
	fi
}


function required_args
{
	exec_name=$0;
	req_args_list=$1; shift

#	echo "exec_name:$exec_name req_args_list:$req_args_list"

	args_missing=$req_args_list

	for (( param_num=$#-1 ; $param_num >= 0 ; param_num=$param_num-1 )) ; do
		#param=${BASH_ARGV[$param_num]#-};
		param=${BASH_ARGV[$param_num]};
		if [ -z "`echo $param | grep '^-'`" ]; then
			echo "ERROR: Invalid argument \"$param\"! Each argument should have the form \"-name value\"" >&2
			exit 1
		fi
		param=${param#-}
		param_num=$(( ${param_num} - 1 ));
		if [[ ${BASH_ARGV[$param_num]} != "UNSET_VARIABLE" ]] ; then
			args_missing=$(
				echo $args_missing | awk -v param=$param '
				{
					sep=""; 
					found=0;
					for(i=1;i<=NF;i++) {
						if($i == param) {
							found = 1;
						} else {
							printf sep $i; 
							sep=" ";
						}
					}
					if (!found) {
						print "ERROR: unknown argument \"-"param"\"" >> "/dev/stderr"
						exit 1
					}
				}'
			)
			if [ $? -ne 0 ]; then
				exit
			fi

			eval "$param=${BASH_ARGV[$param_num]}" ;
		fi
	done

	if [ ! -z "$args_missing" ]; then
		echo "ERROR: Some arguments are missing! (`echo $args_missing | awk '{for(i=1;i<=NF;i++){$i="-"$i} print}'`)" >&2
		echo "Usage: $exec_name ARGUMENTS" >&2
		echo "ARGUMENTS:" >&2
		echo $req_args_list | awk '{for(i=1;i<=NF;i++){print "  -"$i}}' >&2
		exit 1
	fi
}
