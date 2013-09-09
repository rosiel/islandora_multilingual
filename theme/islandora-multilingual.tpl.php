<?php

/**
 * @file
 * This is the template file for the multilingual object
 *
 * @TODO: Add documentation about this file and the available variables
 */
?>
<div class="islandora-multilingual-object islandora">
  <div class="islandora-multilingual-content-wrapper clearfix">
    <?php if (isset($islandora_content)): ?>
      <div class="islandora-multilingual-content">
        <?php print $islandora_content; ?>
      </div>
    <?php endif; ?>
    <div class="islandora-multilingual-sidebar">
    <?php foreach ($brief_metadata as $key => $value): ?>
      <dt class="<?php print $value['class']; ?>">
        <?php print $value['label']; ?>
      </dt>
      <dd class="<?php print $value['class'];?>">
        <?php print $value['value']; ?>
      </dd>
    <?php endforeach; ?>
    <?php if($parent_collections): ?>
        <dt class="collections"><?php print t('In collections:'); ?></dt>
        <dd class="collections"><ul>
          <?php foreach ($parent_collections as $collection): ?>
            <li><?php print l($collection->label, "islandora/object/{$collection->id}"); ?></li>
          <?php endforeach; ?>
        </ul></dd>
    <?php endif; ?>
  </div>
</div>
  <fieldset class="collapsible collapsed islandora-multilingual-metadata">
  <legend><span class="fieldset-legend"><?php print t('Additional information'); ?></span></legend>
    <div class="fieldset-wrapper">
      <dl class="islandora-inline-metadata islandora-multilingual-fields">
        <?php $row_field = 0; ?>
        <?php foreach ($mods_array as $key => $value): ?>
          <dt class="<?php print $value['class']; ?><?php print $row_field == 0 ? ' first' : ''; ?>">
            <?php print $value['label']; ?>
          </dt>
          <dd class="<?php print $value['class']; ?><?php print $row_field == 0 ? ' first' : ''; ?>">
            <?php print $value['value']; ?>
          </dd>
          <?php $row_field++; ?>
        <?php endforeach; ?>
      </dl>
    </div>
  </fieldset>
</div>
