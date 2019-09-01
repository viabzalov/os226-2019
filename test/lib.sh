
cmpout() {
	local base=$1
	shift 1

	"$@" < $base.in | cmp $base.out
}
