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
      <?php print $islandora_view_link; ?>
      <?php print ' | '; ?>
      <?php print $islandora_download_link; ?>
    <?php endif; ?>
  <div class="islandora-multilingual-sidebar">
    <?php if (!empty($dc_array['dc:description']['value'])): ?>
      <h2>This is a description label <?php print $dc_array['dc:description']['label']; ?></h2>
      <p> <?php print $dc_array['dc:description']['value']; ?></p>
    <?php endif; ?>
    <?php if($parent_collections): ?>
      <div>
        <h2><?php print t('In collections'); ?></h2>
        <ul>
          <?php foreach ($parent_collections as $collection): ?>
            <li><?php print l($collection->label, "islandora/object/{$collection->id}"); ?></li>
          <?php endforeach; ?>
        </ul>
      </div>
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
