#!/bin/sh
# USAGE: ./generate_tests.sh 14_4

# Checks graph argument was given.
if [ -z "${1}" ]; then
    echo "Graph was not specified."
    exit 1
fi

templates=$(realpath "./template")
graph=${1}
graph_path=$(realpath graphs/graph_${graph}.txt)
THREADS=( 2 4 8 16 20 )

mkdir -p ${graph}
dir=$(realpath ${graph})
runner=$(realpath "${graph}/runner.sh")
echo '#!/bin/sh' > ${runner}

add_script_to_runner() {
    echo "qrun 20c ${process} ${queue} ${script} >> ${runner}"
    chmod +x ${script}
    echo "qrun 20c ${process} ${queue} ${script}" >> ${runner}
}

generate_serial() {
    sed -E '
/#\$/ s|(-[oe]) \.|\1 '"${dir}"'|;
/arg1 arg2/ c\
'"${env}$(realpath ./build/${executable}) ${graph_path}"'
' <"${templates}/serial_job.sh" >${script}
    add_script_to_runner
}

# Sequential.
executable='sequential'
env=''
process=1
queue='pdp_serial'
script=$(realpath "${dir}/j${graph}_seq.sh")
generate_serial

# Parallel task and data.
executables=( 'parallel_task' 'parallel_data' )
for executable in "${executables[@]}"; do
    abrv=$(echo ${executable} | sed -E 's/(\w).*_(\w).*/\1\2/')
    for thread in "${THREADS[@]}"; do
        script=$(realpath "${dir}/j${graph}_${abrv}_${thread}.sh")
        env="OMP_NUM_THREADS=${thread} "
        generate_serial
    done
done

# Multiprocessing.
executable='multiprocessing'
queue='pdp_long'
processes=( 3 4 )
for process in "${processes[@]}"; do
    for thread in "${THREADS[@]}"; do
        script=$(realpath "${dir}/j${graph}_mp_${process}_${thread}.sh")
        sed -E '
/#\$/ s|(-[oe]) \.|\1 '"${dir}"'|;
/arg1 arg2/ c\
MY_PARALLEL_PROGRAM="'"$(realpath ./build/${executable}) ${graph_path}"'"
/export MY_VARIABLE1/ c\
export OMP_NUM_THREADS='"${thread}"'
' <"${templates}/parallel_job.sh" >${script}
        add_script_to_runner
    done
done
