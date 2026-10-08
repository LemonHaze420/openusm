#pragma once

#include "info_node.h"
#include "mVectorBasic.h"
#include "sound_instance_id.h"

#include <list.hpp>
#include <set>


struct VisemeStream;
struct nglMorphSet;

namespace ai {
struct voice_box_inode;
}

struct ai_lip_sync {
    ai::info_node *owner;
    VisemeStream *resource;
    nglMorphSet *morph;
    char morph_name[32];
    float elapsed;
    mVectorBasic<string_hash> queued_sounds;
    mVectorBasic<float> queued_delays;

    explicit ai_lip_sync(ai::voice_box_inode *voice);
    ~ai_lip_sync();

    bool play(string_hash sound);
    void set_morph_name(const mString &name);
    void queue(string_hash sound, float delay);
    void frame_advance(float elapsed_seconds);
    void stop_all();
    bool sound_start_due() const;
};

namespace ai {

struct speech_request {
    string_hash sound;
    int source_type;
    int interruption;
    int priority;
    unsigned char excluded_teams[15];
    unsigned char padding;
};

struct voice_box_inode : info_node {
    sound_instance_id current_sound;
    int current_priority;
    string_hash pending_sound;
    ai_lip_sync *lip_sync;
    int flags;
    unsigned char teams[15];
    unsigned char padding;
    _std::list<speech_request *> *speech_requests;


    voice_box_inode();

    explicit voice_box_inode(from_mash_in_place_constructor *constructor);
    ~voice_box_inode();
    static void *native_vtable();
    void _unmash(mash_info_struct *info, void *context);
    void _activate_voice(ai_core *core);
    void _frame_advance(Float elapsed_seconds);
    void _destruct_mashed_class();
    void shut_up();
    bool service_speech_request(const speech_request &request);

    bool can_gab() const;
    bool say_file(string_hash sound, int interruption, int priority, const unsigned char *excluded_teams);
    bool say_sound_group(string_hash sound, int interruption, int priority, const unsigned char *excluded_teams);
    bool is_speaking();
    bool say_gab(string_hash sound, int interruption, int priority, const unsigned char *excluded_teams);

    static bool is_any_voice_box_speaking_by_team(int team, const voice_box_inode *except);
    static std::set<voice_box_inode *> *speaking_voice_boxes;
    static std::set<voice_box_inode *> *live_voice_boxes;

private:
    void initialize_voice(mash::allocation_scope scope);
    void finalize_voice(mash::allocation_scope scope);
    void clear_speech_requests();
    bool queue_speech(string_hash sound, int source_type, int interruption, int priority,
                      const unsigned char *excluded_teams);

public:
    void sub_6D7E10(const char *a2);

    inline static const string_hash default_id{int(to_hash("VOICE_BOX"))};
};

}  // namespace ai
