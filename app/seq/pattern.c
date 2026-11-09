/* SPDX-License-Identifier: MPL-2.0 */
/* LOOM-8 pattern store: the plain-data helpers of pattern.h (main loop side; the sequencer only reads). */
#include "pattern.h"

/* the value a component starts with (the notebook p. 139, 0-based): pulse 4, hold 4, multiply 2, velocity 64,
 * ramps 4 iterations, random 4, portamento 40 %, bend down-up, tonality 4 (fifth up), jump to 13, skips: a
 * cycle of 2 (mode 0: the step plays on the cycle's last pass) */
const uint8_t SC_DEFAULT[NSC] = {0, 3, 3, 1, 4, 3, 3, 3, 3, 0, 3, 3, SK_VAL(1, 0), SK_VAL(1, 0), SK_VAL(1, 0), 3};
const char *const GR_NAME[NGROOVE] = {"swing", "lilt", "weave", "stretch", "drunk", "jitter", "accent", "lazy",
                                      "push pull", "sway", "swell"};

static void zero(void *p, uint32_t n)
{
    uint8_t *b = (uint8_t *)p;
    while (n--)
        *b++ = 0;
}

void pattern_init(tpat_t *p)
{
    zero(p, sizeof *p);
    p->len = 16;
    p->scale = TS_1;
    p->groove = 0;                    /* no groove amount */
    p->quant = 100;
    p->nlen = 50;                     /* half a step */
    p->shape = 0;
}

void patterns_init(patterns_t *P)
{
    int t, k;
    zero(P, sizeof *P);
    for (t = 0; t < NSEQT; t++)
        for (k = 0; k < NPATT; k++) {
            pattern_init(&P->pat[t][k]);
            if (t == AUX_BRAIN)       /* a brain step is a beat: a chord progression at four to the bar */
                P->pat[t][k].scale = TS_4;
        }
    for (k = 0; k < NLANE; k++)
        P->lane[k].used = 0;
    P->scene[0].used = 1;             /* scene 1: pattern 1 on every track (pat[] is already 0) */
    P->song.len = 0;
    P->song.loop = 1;
}

/* ----------------------------------------------------------------- lanes --- */
int lane_find(const lane_t *L, int trk, int pat, int param)
{
    int k;
    for (k = 0; k < NLANE; k++)
        if (L[k].used && L[k].trk == trk && L[k].pat == pat && L[k].param == param)
            return k;
    return -1;
}

volatile uint16_t lane_epoch;

int lane_alloc(lane_t *L, int trk, int pat, int param)
{
    int k, i;
    k = lane_find(L, trk, pat, param);
    if (k >= 0)
        return k;
    for (k = 0; k < NLANE; k++)
        if (!L[k].used)
            break;
    if (k == NLANE)
        return -1;
    L[k].trk = (uint8_t)trk;
    L[k].pat = (uint8_t)pat;
    L[k].param = (uint16_t)param;
    for (i = 0; i < NSTEP; i++)
        L[k].val[i] = LOCK_NONE;
    L[k].gen++;                       /* the interrupt sees a new generation: a fresh runtime */
    __asm__ volatile("" ::: "memory");
    L[k].used = 1;
    lane_epoch++;
    return k;
}

void lane_clear(lane_t *L, int k)
{
    if (k >= 0 && k < NLANE) {
        L[k].used = 0;
        lane_epoch++;
    }
}

int lane_count(const lane_t *L)
{
    int k, n = 0;
    for (k = 0; k < NLANE; k++)
        n += L[k].used != 0;
    return n;
}

int step_has_lock(const lane_t *L, int trk, int pat, int step)
{
    int k;
    if (step < 0 || step >= NSTEP)
        return 0;
    for (k = 0; k < NLANE; k++)
        if (L[k].used && L[k].trk == trk && L[k].pat == pat && !lane_is_note(L[k].param) && L[k].val[step] != LOCK_NONE)
            return 1;
    return 0;
}

int step_tune(const lane_t *L, int trk, int pat, int step)
{
    int k;
    if (step < 0 || step >= NSTEP)
        return 0;
    for (k = 0; k < NLANE; k++)
        if (L[k].used && L[k].param == LANE_TUNE && L[k].trk == trk && L[k].pat == pat)
            return L[k].val[step] == LOCK_NONE ? 0 : (int)L[k].val[step] - TUNE_0;
    return 0;
}

int step_vel(const lane_t *L, int trk, int pat, int step, int sound)
{
    int k;
    if (step < 0 || step >= NSTEP)
        return 0;
    for (k = 0; k < NLANE; k++)
        if (L[k].used && L[k].param == LANE_VEL0 + (unsigned)sound && L[k].trk == trk && L[k].pat == pat)
            return L[k].val[step] == LOCK_NONE ? 0 : L[k].val[step];
    return 0;
}

const hit_t *step_hit(const patterns_t *P, int trk, int pat, int step, int sound)
{
    int k;
    if (step < 0 || step >= NSTEP || sound < 0 || sound >= NDRUM)
        return 0;
    for (k = 0; k < NLANE; k++) {
        const lane_t *l = &P->lane[k];
        if (l->used && l->param == LANE_HIT0 + (unsigned)sound && l->trk == trk && l->pat == pat)
            return l->val[step] < NHIT && !hit_plain(&P->hit[l->val[step]]) ? &P->hit[l->val[step]] : 0;
    }
    return 0;
}

/* a random pattern's pitch-class pool: val[0] the low eight bits, val[1] the top four (one lane for the
 * pattern, not a per-step lane) */
uint16_t lane_pool_get(const lane_t *L, int trk, int pat)
{
    int k = lane_find(L, trk, pat, LANE_POOL);
    return k < 0 ? 0u : (uint16_t)(L[k].val[0] | (uint16_t)(L[k].val[1] & 0x0Fu) << 8);
}

int lane_pool_set(lane_t *L, int trk, int pat, uint16_t mask)
{
    int k = lane_find(L, trk, pat, LANE_POOL);
    mask &= 0x0FFFu;
    if (!mask) {                      /* an empty pool: no lane at all */
        if (k >= 0)
            lane_clear(L, k);
        return 0;
    }
    if (k < 0 && (k = lane_alloc(L, trk, pat, LANE_POOL)) < 0)
        return -1;
    L[k].val[0] = (uint8_t)mask;
    L[k].val[1] = (uint8_t)(mask >> 8);
    return 0;
}

/* a track leaves the random player: its pools (one lane per pattern) go, so no lane stays taken that nothing shows */
void lane_pool_free(lane_t *L, int trk)
{
    int k;
    for (k = 0; k < NLANE; k++)
        if (L[k].used && L[k].trk == trk && lane_is_pool(L[k].param))
            lane_clear(L, k);
}

void step_clear_locks(lane_t *L, int trk, int pat, int step)
{
    int k;
    if (step < 0 || step >= NSTEP)
        return;
    for (k = 0; k < NLANE; k++)
        if (L[k].used && L[k].trk == trk && L[k].pat == pat && !lane_is_note(L[k].param))
            L[k].val[step] = LOCK_NONE;
}

void pattern_clear_locks(lane_t *L, int trk, int pat)
{
    int k;
    for (k = 0; k < NLANE; k++)
        if (L[k].used && L[k].trk == trk && L[k].pat == pat && !lane_is_note(L[k].param))
            L[k].used = 0;
}

/* COND_AB + k -> A:B in the order 1:2 2:2 1:3 2:3 3:3 1:4 ... 8:8 (X0X's) */
void cond_ab(int k, int *a, int *b)
{
    int n = 2;
    if (k < 0)
        k = 0;
    if (k >= COND_NAB)
        k = COND_NAB - 1;
    while (k >= n) {
        k -= n;
        n++;
    }
    *a = k + 1;
    *b = n;
}

/* ----------------------------------------------------------------- steps --- */
void step_set_notes(step_t *st, const uint8_t *notes, int n, int vel)
{
    int i;
    for (i = 0; i < SN; i++)
        st->note[i] = i < n ? notes[i] : 0;
    st->vel = (uint8_t)(vel < 1 ? 1 : vel > 127 ? 127 : vel);
}

void step_toggle_note(step_t *st, int note)
{
    int i;
    if (note <= 0 || note > 127)
        return;
    for (i = 0; i < SN; i++)
        if (st->note[i] == note) {    /* there: remove it, close the gap */
            for (; i < SN - 1; i++)
                st->note[i] = st->note[i + 1];
            st->note[SN - 1] = 0;
            return;
        }
    for (i = 0; i < SN; i++)
        if (!st->note[i]) {
            st->note[i] = (uint8_t)note;
            if (!st->vel)
                st->vel = 100;
            return;
        }
}

int step_empty(const step_t *st)
{
    int i;
    for (i = 0; i < SN; i++)
        if (st->note[i])
            return 0;
    return 1;
}

/* the kit's steps (seq.h): the interrupt reads note[0] and note[1] apart, so a change between them plays at
 * most one step half old */
void kit_set(step_t *st, uint32_t mask)
{
    mask &= KIT_ALL;
    st->note[2] = st->note[3] = 0;
    st->note[1] = (uint8_t)(mask >> 8);
    st->note[0] = (uint8_t)(mask & 255u);
}
void kit_toggle(step_t *st, int sound)
{
    if (sound < 1 || sound > NDRUM)
        return;
    kit_set(st, kit_mask(st) ^ 1u << (sound - 1));
    if (!st->vel)
        st->vel = 100;
}
int kit_add(step_t *st, int sound)
{
    if (sound < 1 || sound > NDRUM || kit_has(st, sound))
        return 0;
    kit_set(st, kit_mask(st) | 1u << (sound - 1));
    return 1;
}
int kit_first(const step_t *st)
{
    uint32_t m = kit_mask(st);
    return m ? __builtin_ctz(m) + 1 : 0;
}

