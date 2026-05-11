get_mnist() {
    if [ ! -d "${SCRIPT_DIR}/dataset"]; then
        mkdir "${SCRIPT_DIR}/dataset"
    fi
    curl -L -o "${SCRIPT_DIR}/dataset/minst-dataset.zip"\
    https://www.kaggle.com/api/v1/datasets/download/hojjatk/mnist-dataset
    unzip "${SCRIPT_DIR}/dataset/minst-dataset.zip" 
}

get_mnist