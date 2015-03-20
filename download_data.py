import os
import tempfile
import urllib2
import urlparse
import tarfile
import contextlib
import shutil

VOC_VAL_URL = 'http://pascallin.ecs.soton.ac.uk/challenges/VOC/voc2007/VOCtrainval_06-Nov-2007.tar'
VOC_TEST_URL = 'http://pascallin.ecs.soton.ac.uk/challenges/VOC/voc2007/VOCtest_06-Nov-2007.tar'

CNN_MEAN = 'http://www.robots.ox.ac.uk/~vgg/software/deep_eval/releases/bvlc/VGG_mean.binaryproto'
CNN_PROTO = 'http://www.robots.ox.ac.uk/~vgg/software/deep_eval/releases/bvlc/VGG_CNN_M_128_deploy.prototxt'
CNN_MODEL = 'http://www.robots.ox.ac.uk/~vgg/software/deep_eval/releases/bvlc/VGG_CNN_M_128.caffemodel'

NEG_IMAGES = 'http://www.robots.ox.ac.uk/~vgg/software/deep_eval/releases/neg_images.tar'

@contextlib.contextmanager
def make_temp_directory():

    temp_dir = tempfile.mkdtemp()
    yield temp_dir
    shutil.rmtree(temp_dir)

def download_url(url, fname):

    data = urllib2.urlopen(url)
    with open(fname, 'wb') as fp:
        fp.write(data.read())

    return fname

def prepare_config_proto(base_path):

    orig_file = 'config.prototxt'
    tmp_file = 'config.prototxt.new'

    with open(orig_file, 'r') as fp_r:
        with open(tmp_file, 'w') as fp_w:
            for line in fp_r:
                fp_w.write(line.replace('<BASE_DIR>', base_path))

    os.remove(orig_file)
    shutil.move(tmp_file, orig_file)

def download_voc_data(target_path):

    if not os.path.exists(target_path):
        os.makedirs(target_path)

    with make_temp_directory() as temp_dir:

        urls = {'val': VOC_VAL_URL, 'test': VOC_TEST_URL}
        fnames = {}
        for set, url in urls.iteritems():
            print 'Downloading %s: %s...' % (set, url)
            fnames[set] = download_url(url, os.path.join(temp_dir, set + '.tar'))

        for set, fname in fnames.iteritems():
            print 'Extracting %s: %s...' % (set, fname)
            with tarfile.open(fname) as tar:
                tar.extractall(target_path)

def download_neg_images(target_path):

    if not os.path.exists(target_path):
        os.makedirs(target_path)

    with make_temp_directory() as temp_dir:

        print 'Downloading %s...' % NEG_IMAGES
        fname = download_url(NEG_IMAGES, os.path.join(temp_dir, NEG_IMAGES.split('/')[-1].split('#')[0].split('?')[0]))

        print 'Extracting %s...' % fname
        with tarfile.open(fname) as tar:
            tar.extractall(target_path)

def download_models(target_path):

    if not os.path.exists(target_path):
        os.makedirs(target_path)

    with make_temp_directory() as temp_dir:

        urls = [CNN_MEAN, CNN_PROTO, CNN_MODEL]
        fnames = []
        for url in urls:
            print 'Downloading %s...' % url
            fnames.append(download_url(url, os.path.join(temp_dir, url.split('/')[-1].split('#')[0].split('?')[0])))

        for fname in fnames:
            print 'Copying %s...' % fname
            shutil.copyfile(fname, os.path.join(target_path, os.path.split(fname)[1]))

if __name__ == "__main__":

    file_dir = os.path.dirname(os.path.realpath(__file__))
    prepare_config_proto(file_dir)

    target_dir = os.path.join(file_dir, 'server_data', 'dset_images')
    download_voc_data(target_dir)

    target_dir = os.path.join(file_dir, 'server_data', 'neg_images')
    download_neg_images(target_dir)

    target_dir = os.path.join(file_dir, 'model_data')
    download_models(target_dir)
