import os
from time import sleep

import matplotlib.pyplot as plt
import matplotlib.image as mpimg

import pyclient

VISUALISE_RESULTS = True    # if TRUE visualise the output ranking
IMAGE_DOWNLOAD_TIMEOUT = 20 # time to wait for images to be downloaded and feats computed

config_file = '/Data/src/cpuvisor-srv/config.prototxt'

# connect to server

client = pyclient.VisorClient(config_file)

# send requests

print 'Getting Query ID...'

query_id = client.start_query()

print 'Query ID is: %s'  % query_id

# ----

print 'Adding Training Samples...'

client.download_trs(query_id, 'car')

print 'Started training sample addition process...'

sleep(IMAGE_DOWNLOAD_TIMEOUT)

# ----

print 'Sending train + rank commands...'

# training and ranking function operates in blocking manner,
# stopping processing of all outstanding training images
ranking = client.train_rank_get_ranking(query_id)

ctr = 1;
for ritem in ranking.rlist:
    print '%d: %s (%f)' % (ctr, ritem.path, ritem.score)
    ctr = ctr + 1
    if ctr > 10:
        break

print 'Retrieved %d results from page %d of %d of complete ranking' % \
    (len(ranking.rlist), ranking.page, ranking.page_count)

# ----

# following line shows how we could get subsequent ranking page from backend service:
# ranking = client.get_ranking(query_id, 2)

# once we're done, we can free the query in the backend to save memory
client.free_query(query_id)

# visualise ranked results

if VISUALISE_RESULTS:

    rlist_plt = ranking.rlist[:3*6]

    fig, axes = plt.subplots(3, 6, figsize=(12, 6),
                             subplot_kw={'xticks': [], 'yticks': []})
    fig.subplots_adjust(hspace=0.3, wspace=0.05)

    for ax, ritem in zip(axes.flat, rlist_plt):
        im = mpimg.imread(os.path.join(client.config.preproc_config.dataset_im_base_path, ritem.path))
        ax.imshow(im)
        ax.set_title(ritem.score)

    plt.show()
