#include "meta.h"
#include "../layout/layout.h"
#include "../coding/coding.h"


/* VS - VagStream from Square games [Final Fantasy X (PS2) voices, Unlimited Saga (PS2) voices] */
VGMSTREAM * init_vgmstream_vs_ffx(STREAMFILE *streamFile) {
    VGMSTREAM * vgmstream = NULL;
    int channel_count, loop_flag, pitch;
    off_t start_offset;


    /* checks */
    /* .vs: header id (probably ok like The Bouncer's .vs, very similar) */
    if (!check_extensions(streamFile, "vs"))
        goto fail;
    if (read_32bitBE(0x00,streamFile) != 0x56530000) /* "VS\0\0" */
        goto fail;

    /* 0x04: null (flags? used in SVS) */
    /* 0x08: block number */
    /* 0x0c: blocks left in the subfile */
    pitch = read_32bitLE(0x10,streamFile); /* usually 0x1000 = 48000 */
    /* 0x14: volume, usually 0x64 = 100 but be bigger/smaller (up to 128?) */
    /* 0x18: null */
    /* 0x1c: null */

    loop_flag = 0;
    channel_count = 1;
    start_offset = 0x00;


    /* build the VGMSTREAM */
    vgmstream = allocate_vgmstream(channel_count,loop_flag);
    if (!vgmstream) goto fail;

    vgmstream->meta_type = meta_VS_FFX;
    vgmstream->sample_rate = round10((48000 * pitch) / 4096); /* needed for rare files */
    vgmstream->coding_type = coding_PSX;
    vgmstream->layout_type = layout_blocked_vs_ffx;

    if (!vgmstream_open_stream(vgmstream,streamFile,start_offset))
        goto fail;

    /* calc num_samples */
    {
        vgmstream->next_block_offset = start_offset;
        do {
            block_update(vgmstream->next_block_offset,vgmstream);
            vgmstream->num_samples += ps_bytes_to_samples(vgmstream->current_block_size, 1);
        }
        while (vgmstream->next_block_offset < get_streamfile_size(streamFile));
        block_update(start_offset, vgmstream);
    }

    return vgmstream;

fail:
    close_vgmstream(vgmstream);
    return NULL;
}
