import os
import argparse

def generate_imagelist(dest_file, root_dir, image_exts=['jpg']):

    # compile image list
    with open(dest_file, 'w') as f:
        for root, dirs, files in os.walk(root_dir, followlinks=True):

            rel_root = os.path.relpath(root, root_dir)
            image_paths = [os.path.join(rel_root, x) for x in files
                           if os.path.splitext(x)[1] in image_exts]

            print 'Processing directory %s (%d images)...' % (rel_root, len(image_paths))

            for image_path in image_paths:
                f.write('%s\n' % image_path)


if __name__ == "__main__":

    # parse input arguments
    parser = argparse.ArgumentParser(description='Generate imagelist from directory')
    parser.add_argument('root_dir', type=str,
                        help='Root directory to use as base for image search')
    parser.add_argument('dest_file', type=str,
                        help='Output imagelist file')
    parser.add_argument('--exts', dest='image_exts', type=str, default=['jpg'], nargs='+',
                        help='Extensions of images to use')
    args = parser.parse_args()

    args.image_exts = ['.%s' % x if x[0] != '.' else x for x in args.image_exts]

    generate_imagelist(args.dest_file, args.root_dir, args.image_exts)
